import re

with open('CS3113/Level1.h', 'r', encoding='utf-8') as f:
    content = f.read()

match = re.search(r'unsigned int mLevelData\[.*?\] = \{(.*?)\};', content, re.DOTALL)
if match:
    data_str = match.group(1)
    numbers = [int(x.strip()) for x in re.findall(r'\d+', data_str)]
    
    print(f"Total numbers: {len(numbers)}")
    print(f"Expected: 50 * 25 = {50 * 25}")
    
    if len(numbers) == 50 * 25:
        print("✓ Map data is complete!")
    else:
        print(f"✗ ERROR: Missing {50 * 25 - len(numbers)} elements")
    
    rows = [numbers[i:i+50] for i in range(0, len(numbers), 50)]
    print(f"\nNumber of complete rows: {len(rows)}")
    
    wrong_rows = [i for i, row in enumerate(rows) if len(row) != 50]
    if wrong_rows:
        print(f"✗ ERROR: Rows with wrong length: {wrong_rows}")
    else:
        print("✓ All rows have correct length (50 elements)")

