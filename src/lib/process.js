Promise.all([
  new Promise(resolve => {
    document.addEventListener('DOMContentLoaded', function onContentLoaded () {
      document.removeEventListener('DOMContentLoaded', onContentLoaded)
      resolve()
    })
  })
]).then(() => {
  // TODO
  function generateMessageUid() {
    return 1
  }

  // NaCl module reference
  const naclModule = document.getElementById('nacl_module')

  if (naclModule === null) {
    throw new Error('NaCl module is not supported')
  }

  function postMessage(type, data) {
    const id = generateMessageUid()

    console.log('posting nacl message', id, type, data)

    naclModule.postMessage({
      id: id,
      type: type,
      data
    })
  }

  function handleMessage (event) {
    console.log('handle nacl message', event.data);
  }

  function openChatWindow () {
    chrome.app.window.create('chat.html', {
      id: 'chat',
      frame: {
        color: '#000000'
      },
      outerBounds: { minWidth: 360, width: 360, minHeight: 640, height: 640 }
    }, (appWindow) => {
      console.log('opened', appWindow)

      // TODO: Remove dummy code
      const doc = appWindow.contentWindow.document

      doc.addEventListener('DOMContentLoaded', function onAppContentLoaded () {
        doc.removeEventListener('DOMContentLoaded', onAppContentLoaded)

        const buttonElement = doc.getElementById('button')

        buttonElement.addEventListener('click', () => {
          buttonElement.setAttribute('disabled', 1)
          buttonElement.innerHTML = 'Running...'

          postMessage('CONFIGURE', {
            foo: 'bar'
          })
        })
      })
    })
  }

  naclModule.addEventListener('message', handleMessage, false)

  chrome.app.runtime.onLaunched.addListener(openChatWindow)
})
