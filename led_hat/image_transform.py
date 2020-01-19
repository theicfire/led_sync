import cv2
  
# Save image in set directory 
# Read RGB image 
img = cv2.imread('sweet.png')  
full = []
for i in range(len(img)):
    row = []
    for j in range(len(img[0])):
        row.append('{{{},{},{}}}'.format(img[i][j][0], img[i][j][1], img[i][j][2]))
    full.append('{{{}}}'.format(',\n'.join(row)))
print('uint8_t first_image[300][300][3] PROGMEM = {{{}}};'.format(',\n'.join(full)))


