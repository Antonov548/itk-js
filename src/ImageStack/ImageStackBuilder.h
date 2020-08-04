#ifndef ImageStackBuilder_H
#define ImageStackBuilder_H

#include "IDataDestination.h"

#include <memory>
#include <string>

namespace itkjs
  {
  namespace ImageStack
    {
      
    ////////////////////////////////////////////////////////////////////////
    
    class IAsyncDataLoader;
    class IDataDecoder;
      
    ////////////////////////////////////////////////////////////////////////
          
    class ImageStackBuilder
      {
      public:
        typedef std::function<void(void)> TOnReadyCallback;
        typedef std::function<void(const char*)> TOnFailedCallback;
        typedef std::function<void(const char*)> TStatusCallback;
        typedef std::function<void(int, unsigned, unsigned)> TOnLoadingProgressCallback;
        typedef std::function<void(unsigned, unsigned)> TOnDecodingProgressCallback;
        
      public:
        ImageStackBuilder();
        
        void OnLoadingProgress(TOnLoadingProgressCallback i_callback);
        void OnDecodingProgress(TOnDecodingProgressCallback i_callback);
        
        void LoadDataAsync(
          const std::string& i_header_url,
          const std::string& i_data_url,
          TOnReadyCallback i_on_ready_callback,
          TOnFailedCallback i_on_failed_callback,
          TStatusCallback i_status_callback);
        
      private:
        class _DataDestinationProxy;
        
        struct _Header
          {
          unsigned component_size;
          unsigned dimensions[3];
          float spacing[3];
          float origin[3];
          float directions[3];
          };
        
      private:        
        friend class _DataDestinationProxy;
        unsigned _GetComponentSize(const std::string& i_component_type_str) const;
        void _ProcessLoadedHeader(void* ip_buffer, unsigned i_buffer_size);
        void _OnHeaderLoadingFailed(const char* ip_description);
        void _ProcessLoadedData(void* ip_buffer, unsigned i_buffer_size);
        void _OnDataLoadingFailed(const char* ip_description);
        
      private:
        std::unique_ptr<_Header> mp_header;
        TOnReadyCallback m_on_ready_callback;
        TOnFailedCallback m_on_failed_callback;
        TStatusCallback m_status_callback;
        std::string m_data_url;
        std::unique_ptr<IAsyncDataLoader> mp_header_loader;
        std::unique_ptr<IAsyncDataLoader> mp_data_loader;
        std::unique_ptr<IDataDecoder> mp_data_decoder;
        std::unique_ptr<IDataDestination> mp_header_processor;
        std::unique_ptr<IDataDestination> mp_data_processor;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    class ImageStackBuilder::_DataDestinationProxy : public IDataDestination
      {
      public:
        typedef void(ImageStackBuilder::*TProcessor)(void*, unsigned);
        typedef void(ImageStackBuilder::*TOnFailed)(const char*);
        
      public:
        _DataDestinationProxy(ImageStackBuilder& ir_parent, TProcessor ip_processor, TOnFailed ip_on_failed);
        
        // IDataDestination
        virtual void ProcessLoadedData(void* ip_buffer, unsigned i_buffer_size) override;
        virtual void OnDataLoadingFailed(const char* ip_description) override;
        
      private:
        ImageStackBuilder& mr_parent;
        TProcessor mp_processor;
        TOnFailed mp_on_failed;
      };
      
    ////////////////////////////////////////////////////////////////////////
      
    }  
  }

#endif