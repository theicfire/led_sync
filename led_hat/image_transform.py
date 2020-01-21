import cv2
  

def lower_brightness(img, factor):
    hsvImg = cv2.cvtColor(img,cv2.COLOR_BGR2HSV)

    # decreasing the V channel by a factor from the original
    hsvImg[...,2] = hsvImg[...,2]*factor

    return cv2.cvtColor(hsvImg,cv2.COLOR_HSV2BGR)
    # cv2.imwrite('visuals/out.png', cv2.cvtColor(hsvImg,cv2.COLOR_HSV2RGB))


def main():
    img = cv2.imread('visuals/test_colors.png')
    img = lower_brightness(img, .2)
    full = []
    for i in range(len(img)):
        row = []
        # print(i, img[i][299], img[i][299])
        for j in range(len(img[0])):
            row.append('{{{},{},{}}}'.format(img[i][j][2], img[i][j][1], img[i][j][0]))
        full.append('{{{}}}'.format(',\n'.join(row)))
    print('#pragma once')
    print('uint8_t image_merge[{}][{}][3] PROGMEM = {{{}}};'.format(img.shape[0], img.shape[1], ',\n'.join(full)))



main()
