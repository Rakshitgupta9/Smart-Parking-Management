from flask import Flask, request, jsonify, Response
import cv2
import os
import easyocr
from werkzeug.utils import secure_filename
from datetime import datetime
import json
from flask_cors import CORS

app = Flask(__name__)
CORS(app)
app.config['UPLOAD_FOLDER'] = 'static'

# Initialize EasyOCR reader once
reader = easyocr.Reader(['en'])

@app.route('/upload', methods=['POST'])
def upload_image():
    if 'image_file' not in request.files:
        error_response = json.dumps({"error": "No file part"})
        return Response(
            error_response,
            mimetype='application/json',
            status=400,
            headers={'Content-Length': str(len(error_response))}
        )

    file = request.files['image_file']

    if file.filename == '':
        error_response = json.dumps({"error": "No selected file"})
        return Response(
            error_response,
            mimetype='application/json',
            status=400,
            headers={'Content-Length': str(len(error_response))}
        )

    # Create a unique filename
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S%f")
    original_filename = secure_filename(file.filename)
    filename = f"{timestamp}_{original_filename}"
    filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
    
    # Save uploaded file
    file.save(filepath)

    # Read image
    img = cv2.imread(filepath)
    if img is None:
        app.logger.error(f"Failed to read image: {filepath}")
        error_response = json.dumps({"error": "Could not process the image"})
        return Response(
            error_response,
            mimetype='application/json',
            status=500,
            headers={'Content-Length': str(len(error_response))}
        )

    # OCR processing
    result = reader.readtext(img)

    # Base URL dynamically generated
    base_url = request.host_url.rstrip('/')

    if result:
        # Get the most confident prediction
        most_confident = max(result, key=lambda x: x[2])
        bbox = most_confident[0]
        text = most_confident[1]

        # Draw bounding box and text
        top_left = tuple(map(int, bbox[0]))
        bottom_right = tuple(map(int, bbox[2]))
        cv2.rectangle(img, top_left, bottom_right, (0, 255, 0), 2)
        cv2.putText(img, text, (top_left[0], top_left[1] - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)

        annotated_filename = f"annotated_{filename}"
        annotated_filepath = os.path.join(app.config['UPLOAD_FOLDER'], annotated_filename)
        cv2.imwrite(annotated_filepath, img)

        response_data = {
            "data": {
                "message": "ANPR successful",
                "number_plate": text,
                "view_image": f"{base_url}/static/{annotated_filename}"
            }
        }
    else:
        response_data = {
            "data": {
                "message": "No number plate detected",
                "number_plate": None,
                "view_image": f"{base_url}/static/{filename}"
            }
        }

    json_data = json.dumps(response_data)
    return Response(
        json_data,
        mimetype='application/json',
        status=200,
        headers={'Content-Length': str(len(json_data))}
    )

if __name__ == '__main__':
    if not os.path.exists('static'):
        os.makedirs('static')
    app.run(host='0.0.0.0', port=5000, debug=True)
