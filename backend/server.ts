import { createServer } from 'http';
import { readFileSync } from 'fs';
import { WebSocketServer } from 'ws';

type WSCB = {
  [event in string]: ((data: any) => any)[]
}

// type WSCBs = WSCB[] 

export async function createServers(wsCB: WSCB, cb: (req: any, res: any) => Promise<any>) {
  return new Promise((resolve, reject) => {
    const server = createServer(async (req, res) => {
      const data = await cb(req, res);
    });
    
    function createWSServer(wsCB: WSCB) {
      const wss = new WebSocketServer({ server });
      wss.on('connection', function connection(ws) {
        console.log('WS connected');
        ws.on('error', console.error);
      
        ws.on('message', function message(data) {
          const mess = data.toString();
          const fc = mess.indexOf(',');
          const event = mess.slice(0, fc)
          const message = mess.slice(fc);
          if (wsCB[event]) {
            wsCB[event].map(el=>{el(message)});
          }
          console.log('received: %s', data);
        });
      
        ws.send('something');
      });
      resolve({
        broadcast: (message: any) => {
          wss.clients.forEach(function each(client) {
            if (client.readyState === WebSocket.OPEN) {
              client.send(message);
            }
          });
        }
      });
    }
    createWSServer(wsCB);
    server.listen(8080);
  })
}  
