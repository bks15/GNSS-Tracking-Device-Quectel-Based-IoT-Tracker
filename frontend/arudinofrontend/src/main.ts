import { MyMap } from './map';
import './style.css'

const debugElement = document.getElementById("debug");

function createDebugElement(data: string) {
  const ele = document.createElement('div');
  ele.className = 'debugData';
  ele.textContent = data;
  debugElement?.children[1]?.append(ele);
}

function handleWSMessage(message: any) {
  createDebugElement(message.data);
  try {
    const json = JSON.parse(message.data);
    if (json?.data?.['lat'], json?.data?.['lng']) {
      MyMap.getInstance().addMarker(json.data);
      // addMarker(json);
    }
  } catch(err) {
    // console.log(err)
    console.log('NO CORD')
  }
}

async function connectWebSocket() {
  // console.log(import.meta.env)
  const wsConnString = `ws://${import.meta.env['VITE_BACKEND_HOST']}:${import.meta.env['VITE_BACKEND_PORT']}`;
  // console.log({wsConnString})
  const ws = new WebSocket(wsConnString);
  ws.onopen = (eve: any) => {
    console.log('Connection opened', eve);
  }
  ws.onmessage = (message: any) => {
    console.log('GOT MESSAGE', message.data);
    handleWSMessage(message);
  }
}

window.addEventListener('load', async () => {
  await MyMap.getInstance().init();
  connectWebSocket();
})