let extensionToIO = {}

extensionToIO['dcm'] = 'itkGDCMImageIOJSBinding'
extensionToIO['DCM'] = 'itkGDCMImageIOJSBinding'

extensionToIO['mha'] = 'itkMetaImageIOJSBinding'
extensionToIO['mhd'] = 'itkMetaImageIOJSBinding'

extensionToIO['nrrd'] = 'itkNrrdImageIOJSBinding'
extensionToIO['NRRD'] = 'itkNrrdImageIOJSBinding'
extensionToIO['nhdr'] = 'itkNrrdImageIOJSBinding'
extensionToIO['NHDR'] = 'itkNrrdImageIOJSBinding'

extensionToIO['png'] = 'itkPNGImageIOJSBinding'
extensionToIO['PNG'] = 'itkPNGImageIOJSBinding'

extensionToIO['vtk'] = 'itkVTKImageIOJSBinding'
extensionToIO['VTK'] = 'itkVTKImageIOJSBinding'

module.exports = extensionToIO
