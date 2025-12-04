width, height = 64, 40
data = []
for row in range(height):
    for col in range(width):
        if row == 0 or row == height-1 or col == 0 or col == width-1:
            data.append(1)  # Border
        elif 2 <= row <= 12 and 2 <= col <= 12:  # Top-left room
            if row == 2 or row == 12 or col == 2 or (col == 12 and (row < 6 or row > 8)):
                data.append(1)
            else:
                data.append(2)
        elif 2 <= row <= 12 and 51 <= col <= 61:  # Top-right room
            if row == 2 or row == 12 or col == 61 or (col == 51 and (row < 6 or row > 8)):
                data.append(1)
            else:
                data.append(2)
        elif 27 <= row <= 37 and 2 <= col <= 12:  # Bottom-left room
            if row == 27 or row == 37 or col == 2 or (col == 12 and (row < 30 or row > 32)):
                data.append(1)
            else:
                data.append(2)
        elif 27 <= row <= 37 and 51 <= col <= 61:  # Bottom-right room
            if row == 27 or row == 37 or col == 61 or (col == 51 and (row < 30 or row > 32)):
                data.append(1)
            else:
                data.append(2)
        elif row == 7 and 13 <= col <= 50:  # Top corridor
            data.append(2)
        elif row == 31 and 13 <= col <= 50:  # Bottom corridor
            data.append(2)
        elif col == 7 and 13 <= row <= 26:  # Left vertical
            data.append(2)
        elif col == 56 and 13 <= row <= 26:  # Right vertical
            data.append(2)
        elif 13 <= row <= 26 and 13 <= col <= 50:  # Center
            if (row == 19 and 28 <= col <= 35) or (col == 31 and 20 <= row <= 25):
                data.append(3)
            else:
                data.append(2)
        else:
            data.append(2)

# Format as C array
print('unsigned int mLevelData[LEVEL1_WIDTH * LEVEL1_HEIGHT] = {')
for i in range(0, len(data), width):
    row_data = data[i:i+width]
    line = ', '.join(map(str, row_data))
    print(f'        {line}' + (',' if i < len(data) - width else ''))
print('    };')

