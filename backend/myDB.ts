import { MongoClient } from 'mongodb';

const url = 'mongodb://mongodb:27017';

export async function startMongo() {
  const mongoClient = new MongoClient(url);
  
  await mongoClient.connect();
  const db = mongoClient.db('arduino_db');
  const coll = db.collection('sensor_data');
  
  // const insResult = await coll.insertOne({'nice': 'world', time: +(new Date())});
  
  // console.log({insResult});
  
  // const readResult = await coll.find().toArray();
  // console.log({readResult});

  return coll;
}



