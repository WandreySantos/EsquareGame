from PIL import Image

def img_to_hex_matrix(image_path, threshold=128):
    # Abre e converte a imagem pra escala de cinza
    img = Image.open(image_path).convert('L')
    
    # Redimensiona pra 16x16
    img = img.resize((16, 16))
    
    # Cria a lista pra armazenar as linhas em hexadecimal
    hex_rows = []

    print("🧩 Matriz 16x16 em hexadecimal:\n")
    for y in range(16):
        row_value = 0
        for x in range(16):
            pixel = img.getpixel((x, y))
            bit = 1 if pixel < threshold else 0  # preto = LED ligado
            row_value = (row_value << 1) | bit
        hex_rows.append(row_value)
        print(f"0x{row_value:04X},", end='')
        if y % 4 == 3:
            print()
    
    print("\n\n✅ Copie o array abaixo para seu código C++:\n")
    print("const uint16_t image16x16[16] = {")
    for row in hex_rows:
        print(f"  0x{row:04X},")
    print("};")

# Exemplo de uso:
# img_to_hex_matrix('meu_desenho.png')

if __name__ == "__main__":
    caminho = input("Digite o caminho da imagem (ex: desenho.png): ")
    img_to_hex_matrix(caminho)
