#include "HEVCDataDecoder.h"

#include "IDataDestination.h"
#include "NullProgressReporter.h"
#include "Header.h"
#include "MemoryManagement.h"

#include <libde265/de265.h>

namespace
  {
  void _Checked(de265_error i_err)
    {
    if (i_err != DE265_OK)
      throw std::runtime_error(std::string("Decoder error: ") + de265_get_error_text(i_err));
    }
  }

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
            
    HEVCDataDecoder::HEVCDataDecoder(IDataDestination& ir_data_destination)
      : mr_data_destination(ir_data_destination)
      , mp_progress_reporter(std::unique_ptr<IProgressReporter>(new NullProgressReporter()))
      {
      }
      
    void HEVCDataDecoder::SetProgressReporter(std::unique_ptr<IProgressReporter>&& ip_reporter)
      {
      mp_progress_reporter = std::move(ip_reporter);
      }
      
    void HEVCDataDecoder::DecodeData(const Header& i_header, TUniqueMallocPtr&& ip_data_buffer, unsigned i_data_buffer_size) const
      {
      try
        {
        const unsigned image_buffer_size = i_header.dimensions[0] * i_header.dimensions[1] * i_header.dimensions[2] * i_header.component_size;
        TUniqueBufferPtr p_image_buffer(new uint8_t[image_buffer_size]);
          
        uint8_t* p_buffer = reinterpret_cast<uint8_t*>(p_image_buffer.get());
        uint16_t* p_buffer16 = reinterpret_cast<uint16_t*>(p_image_buffer.get());

        std::unique_ptr<de265_decoder_context, decltype(&de265_free_decoder)> p_ctx(de265_new_decoder(), de265_free_decoder);
      
        uint8_t* p_input_buffer = reinterpret_cast<uint8_t*>(ip_data_buffer.get());
        unsigned remaining = i_data_buffer_size;
        unsigned count = 4096;

        unsigned num_frames = 0;
        bool stop = false;
        while (!stop)
          {
          // read a chunk of input data
          if (remaining <= count)
            {
            count = remaining;
            stop = true;
            }
          uint8_t* p_chunk = p_input_buffer;
          p_input_buffer += count;
          remaining -= count;

          // push input data chunk to decoder
          _Checked(de265_push_data(p_ctx.get(), p_chunk, count, 0, 0));              
          if (stop)
            _Checked(de265_flush_data(p_ctx.get()));

          int more = 1;
          while (more != 0)
            {
            // decode some more
            de265_error err = de265_decode(p_ctx.get(), &more);
            if (err == DE265_ERROR_WAITING_FOR_INPUT_DATA)
              break; // another chunk is required, breaking to outer loop to get it
            else
              _Checked(err);

            // get image
            const de265_image* p_img = de265_get_next_picture(p_ctx.get());
            if (!p_img)
              continue;

            int width = de265_get_image_width(p_img, 0);
            int height = de265_get_image_height(p_img, 0);
            if ((width != i_header.dimensions[0]) || (height != i_header.dimensions[1]))
              throw std::runtime_error("Invalid input data");

            de265_chroma chroma = de265_get_chroma_format(p_img);
            if (chroma != de265_chroma_mono)
              throw std::runtime_error("Invalid input data");

            int stride_y;
            const uint8_t* p_y = de265_get_image_plane(p_img, 0, &stride_y);
            if (!p_y)
              throw std::runtime_error("Invalid input data");

            int bpp_y = de265_get_bits_per_pixel(p_img, 0);
            if (bpp_y > 8)
              {
              if ((bpp_y != 12) || i_header.component_size != 2)
                throw std::runtime_error("Invalid input data");
              int bpp_y_diff = 16 - bpp_y;
              const uint16_t* p_y16 = reinterpret_cast<const uint16_t*>(p_y);
              for (unsigned i = 0, sz = width * height; i < sz; ++i)
                {
                *p_buffer16 = *p_y16 << bpp_y_diff;
                ++p_buffer16;
                ++p_y16;
                }
              }
            else
              {
              if ((bpp_y != 8) || i_header.component_size != 1)
                throw std::runtime_error("Invalid input data");
              for (unsigned i = 0, sz = width * height; i < sz; ++i)
                {
                *p_buffer = *p_y;
                ++p_buffer;
                ++p_y;
                }
              }

            ++num_frames;
            mp_progress_reporter->OnProgress(num_frames, i_header.dimensions[2]);
            }
          }

        if (num_frames != i_header.dimensions[2])
          throw std::runtime_error("Invalid input data");
        
        mr_data_destination.OnSuccess(std::move(p_image_buffer), image_buffer_size);
        }
      catch (const std::exception& i_e)
        {
        mr_data_destination.OnFailure(i_e.what());
        }
      catch (...)
        {
        mr_data_destination.OnFailure("Exception during data decoding.");
        }
      }
      
    ////////////////////////////////////////////////////////////////////////      
      
    }  
  }