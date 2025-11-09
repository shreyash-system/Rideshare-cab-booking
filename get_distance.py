import sys
import json
import urllib.request
import urllib.parse

def get_distance(origin, destination, api_key):
    base_url = "https://maps.googleapis.com/maps/api/distancematrix/json"
    
    params = {
        'origins': origin,
        'destinations': destination,
        'key': api_key
    }
    
    url = base_url + '?' + urllib.parse.urlencode(params)
    
    try:
        with urllib.request.urlopen(url) as response:
            data = json.loads(response.read().decode())
            
            if data['status'] == 'OK':
                element = data['rows'][0]['elements'][0]
                if element['status'] == 'OK':
                    distance_meters = element['distance']['value']
                    distance_km = distance_meters / 1000.0
                    print(f"{distance_km:.2f}")
                else:
                    print("0")
            else:
                print("0")
    except Exception as e:
        print("0")

if __name__ == "__main__":
    if len(sys.argv) == 4:
        get_distance(sys.argv[1], sys.argv[2], sys.argv[3])
    else:
        print("0")
