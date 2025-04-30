import requests

# Update this with your Flask server IP (or localhost if on same device)
url = 'http://localhost:5000/upload'

# Image file to test
image_path = 'plate.jpg'

# Send POST request
with open(image_path, 'rb') as img_file:
    files = {'image_file': img_file}
    response = requests.post(url, files=files)

print("Status Code:", response.status_code)

try:
    print("Response JSON:")
    print(response.json())
except Exception as e:
    print("Failed to parse JSON:", e)
    print("Raw Response:", response.text)
