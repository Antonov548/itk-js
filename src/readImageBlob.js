import WebworkerPromise from 'webworker-promise'
import PromiseFileReader from 'promise-file-reader'

import config from './itkConfig'

const worker = new window.Worker(config.itkModulesPath + '/WebWorkers/ImageIO.worker.js')
const promiseWorker = new WebworkerPromise(worker)

/**
 * @param: blob Blob that contains the file contents
 * @param: fileName string that contains the file name
 * @param: mimeType optional mime-type string
 */
const readImageBlob = (blob, fileName, mimeType) => {
  return PromiseFileReader.readAsArrayBuffer(blob)
    .then(arrayBuffer => {
      return promiseWorker.postMessage({ operation: 'readImage', name: fileName, type: mimeType, data: arrayBuffer, config: config },
        [arrayBuffer])
    })
}

export default readImageBlob
