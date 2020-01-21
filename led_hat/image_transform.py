import cv2
  
def quantize_img(img):
    for y in range(len(img)):
        for x in range(len(img[0])):
            if (y > 0):
                step_0 = int(img[y-1][x][0]) - int(img[y][x][0])
                if step_0 < 10 and step_0 > -10:
                    img[y][x][0] = img[y-1][x][0]

                step_1 = int(img[y-1][x][1]) - int(img[y][x][1])
                if step_1 < 10 and step_1 > -10:
                    img[y][x][1] = img[y-1][x][1]

                step_2 = int(img[y-1][x][2]) - int(img[y][x][2])
                if step_2 < 10 and step_2 > -10:
                    img[y][x][2] = img[y-1][x][2]

    return img

def plus_one(img):
    for y in range(len(img)):
        for x in range(len(img[0])):
            img[y][x][0] += 3 # 3 maps to 2 in gamma world, 2 seems to be the min to turn the led on
            img[y][x][1] += 3
            img[y][x][2] += 1


def lower_brightness(img, factor):
    hsvImg = cv2.cvtColor(img,cv2.COLOR_BGR2HSV)

    # decreasing the V channel by a factor from the original
    hsvImg[...,2] = hsvImg[...,2]*factor

    return cv2.cvtColor(hsvImg,cv2.COLOR_HSV2BGR)
    # cv2.imwrite('visuals/out.png', cv2.cvtColor(hsvImg,cv2.COLOR_HSV2RGB))


def main():
    img = cv2.imread('visuals/merged.png')
    img = lower_brightness(img, .2)
    plus_one(img)
    quantize_img(img)
    # img = cv2.blur(img, (5, 1))
    cv2.imwrite('visuals/out.png', img)
    full = []
    for i in range(len(img)):
        row = []
        # print('\nrow', i)
        # for j in range(20):
            # print(img[i][j], end=' ')
        for j in range(len(img[0])):
            row.append('{{{},{},{}}}'.format(img[i][j][2], img[i][j][1], img[i][j][0]))
        full.append('{{{}}}'.format(',\n'.join(row)))
    print('#pragma once')
    print('uint8_t image_merge[{}][{}][3] PROGMEM = {{{}}};'.format(img.shape[0], img.shape[1], ',\n'.join(full)))



main()
