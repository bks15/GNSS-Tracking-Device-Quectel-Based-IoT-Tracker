import mqtt from "mqtt";

export async function startMQTT(saveToDB: (data: any) => any) {
  const host = process.env?.['MQTT_HOST'] || '18.136.124.119';
  const port = parseInt(process.env?.['MQTT_PORT'] || 'undefined') || 1883;
  const username = process.env?.['MQTT_USERNAME'] || 'test';
  const password = process.env?.['MQTT_PASSWORD'] || 'test';
  const protocol = process.env?.['MQTT_PROTOCOL'] || 'mqtt';
  console.log('MQTT', {host, port, username, password});
  const client = await mqtt.connectAsync(`${protocol}://${host}`, {port, username, password});

  client.on('error', (err) => {
    console.log('MQTTERROR', err);
  })
  
  await client.subscribeAsync('#');

  // console.log('CLIENT', client);
  
  client.on('message', async (topic: string, message: any) => {
    const data = message.toString();
    console.log('MQTT MESSAGE', {topic, data});
    await saveToDB({topic, data: data});
    // const insResult = await coll.insertOne({data, time: +(new Date())});
    // console.log({insResult});
  })
  
  
  await client.publishAsync('nice', 'hello from node');
}