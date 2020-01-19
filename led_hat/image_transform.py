import cv2
  
# Save image in set directory 
# Read RGB image 
img = cv2.imread('visuals/merged.png')  
full = []
for i in range(len(img)):
    row = []
    for j in range(len(img[0])):
        row.append('{{{},{},{}}}'.format(img[i][j][2], img[i][j][1], img[i][j][0]))
    full.append('{{{}}}'.format(',\n'.join(row)))
print('#pragma once')
print('uint8_t image_merge[{}][{}][3] PROGMEM = {{{}}};'.format(img.shape[0], img.shape[1], ',\n'.join(full)))


