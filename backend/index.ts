import { startMongo } from "./myDB.ts";
import { startMQTT } from "./saveMQTTData.ts";
import { createServers } from "./server.ts";

const eventHandlers = {
  'nice': [
    (data: any) => {

    }
  ]
}

async function httpHandler(req: any, res: any) {

}

const server: any = await createServers(eventHandlers, httpHandler)

const coll = await startMongo();

function nmeaToDegree(data: string) {
  const ddmm = parseFloat(data);
  const deg = parseInt((ddmm / 100).toFixed(0));
  const minutes = ddmm - (deg * 100);
  return deg + (minutes / 60.0);
}

async function saveToDB({topic, data}: {topic: string, data: string}) {
  const res: any = {topic, data};
  if (data.includes(',N,') && data.includes(',E,')) {
    const chunks = data.split(',');
    const lat = nmeaToDegree(chunks[1]);
    const lng = nmeaToDegree(chunks[3]);
    res.data = {lat, lng};
  }
  res['timestamp'] = +(new Date());
  console.log('SAVING TO DB', res);
  server.broadcast(JSON.stringify(res));
}

const mqtt = await startMQTT(saveToDB);

// await client.endAsync();

// console.log(res);

