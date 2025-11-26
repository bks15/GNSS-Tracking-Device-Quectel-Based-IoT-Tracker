const DEFAULT_CENTER = { lat: 37.4211, lng: -122.1234 };

export class MyMap {
  static #instance: MyMap;
  static getInstance() {
    if (!MyMap.#instance) {
      MyMap.#instance = new MyMap();
    }
    return MyMap.#instance;
  }

  Map: any;
  AdvancedMarkerElement: any;
  mapElement: any;
  innerMap: any;

  constructor() {}

  async init () {
    const [{ Map }, { AdvancedMarkerElement }] = await Promise.all([
      // @ts-ignore
      google.maps.importLibrary("maps"),
      // @ts-ignore
      google.maps.importLibrary("marker"),
    ]);
    this.Map = Map;
    this.AdvancedMarkerElement = AdvancedMarkerElement;
    this.mapElement = document.querySelector(
      "gmp-map"
    );
    this.mapElement.center = DEFAULT_CENTER;

    this.innerMap = this.mapElement.innerMap;
  }

  addMarker(pos: {lat: number, lng: number}) {
    console.log('ADDING MARKER', pos);
    
    const priceTag = document.createElement("div");
    priceTag.className = "map-dot";
    
    const marker = new this.AdvancedMarkerElement({
      position: pos || this.mapElement.center,
    });

    marker.append(priceTag);
    this.mapElement.append(marker);
    this.mapElement.center = pos;
  }

}