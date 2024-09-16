import math

def calculate_standard_deviation(numbers):
    # 计算平均值
    mean = sum(numbers) / len(numbers)
    
    # 计算方差
    variance = sum((x - mean) ** 2 for x in numbers) / len(numbers)
    
    # 计算标准差
    std_dev = math.sqrt(variance)
    
    return std_dev

def main():
    print("请输入10个数字，每输入一个数字后按回车键：")
    numbers = []
    
    for i in range(10):
        while True:
            try:
                num = float(input(f"输入第 {i+1} 个数字: "))
                numbers.append(num)
                break
            except ValueError:
                print("输入无效，请输入一个有效的数字。")
    
    std_dev = calculate_standard_deviation(numbers)
    
    print("\n输入的数字为:", numbers)
    print("标准差为: {:.2f}".format(std_dev))

if __name__ == "__main__":
    main()
