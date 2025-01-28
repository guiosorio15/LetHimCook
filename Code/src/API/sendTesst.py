import requests

# Caminho da imagem a ser enviada
image_path = "test.jpg"

# URL do endpoint do servidor
url = "http://127.0.0.1:18080/upload"

# Abrindo a imagem e enviando via multipart/form-data
with open(image_path, "rb") as img_file:
    files = {"image": img_file}
    response = requests.post(url, files=files)

print(f"Resposta do servidor: {response.status_code}, {response.text}")
