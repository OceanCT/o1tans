import subprocess
import re
import os
import csv # 导入 csv 模块用于写入文件

# 定义参数范围
o0numcap = [0, 1/256, 4/256, 0.5, 1.0]
o1numcap = [0, 0.1, 0.2, 1.0]

# 定义一个函数来解析输出并提取所需信息
def parse_output(output):
    """从命令行输出中提取 o1_use_cnt 和 Compression Ratio"""
    o1_use_cnt = None
    compression_ratio = None

    # 使用正则表达式匹配 o1_use_cnt
    # 示例: o1_use_cnt=96, final_header_size=49664
    o1_cnt_match = re.search(r"o1_use_cnt=(\d+)", output)
    if o1_cnt_match:
        o1_use_cnt = int(o1_cnt_match.group(1))

    # 使用正则表达式匹配 Compression Ratio
    # 示例: Compression Ratio: 2.15:1
    ratio_match = re.search(r"Compression Ratio: ([\d\.]+):1", output)
    if ratio_match:
        # 提取比例的第一部分，如 '2.15'
        compression_ratio = float(ratio_match.group(1))

    return o1_use_cnt, compression_ratio

# --- 实验开始 ---
results = []
datasetpath = "datasets/silesia/"
# 确保 datasets/silesia/ 目录存在，如果不存在，则可能需要调整路径或创建
if not os.path.isdir(datasetpath):
    print(f"⚠️ 警告: 目录 '{datasetpath}' 不存在。请检查数据集路径。")
    # 尝试列出当前目录，以便调试
    print(f"当前目录下的文件: {os.listdir(os.getcwd())}")
    exit()

datasetnames = os.listdir(datasetpath)
# 排除 .zip 文件或目录
datasetnames = [name for name in datasetnames if not name.endswith('.zip') and os.path.isfile(os.path.join(datasetpath, name))]

# 检查是否有文件要测试
if not datasetnames:
    print(f"⚠️ 警告: 在 '{datasetpath}' 中没有找到可测试的文件。")

for datasetname in datasetnames:
    # 循环测试 o0numcap 和 o1numcap 的所有组合
    for o0 in o0numcap:
        for o1 in o1numcap:
            # 构建命令。注意：为了兼容 Windows，使用直接调用 EXE 的方式。
            # 如果 fse_testo1.exe 不在 PATH 或当前目录，您可能需要提供完整的路径。
            cmd = f".\\fse_testo1.exe {datasetpath}{datasetname} {o0} {o1}"
            print(f"\n--- Running: {cmd} ---")
            
            try:
                # 使用 subprocess.run() 执行命令并捕获输出
                result = subprocess.run(
                    cmd, 
                    shell=True, 
                    capture_output=True, 
                    text=True, 
                    check=True,  # 如果返回非零状态码，则抛出异常
                    encoding='utf-8', # 显式指定编码，防止中文乱码 (根据实际情况调整编码)
                    timeout=60 # 设置超时时间，防止程序无限期等待
                )

                # 完整的命令行输出
                output = result.stdout
                print(output.strip()) 

                # 解析输出
                o1_cnt, ratio = parse_output(output)

                # 存储结果，现在包含数据集名称
                results.append({
                    "dataset": datasetname,
                    "o0": o0,
                    "o1": o1,
                    "o1_use_cnt": o1_cnt,
                    "ratio": ratio
                })
                
                # 打印提取的关键信息
                print(f"✅ Extracted: o1_use_cnt={o1_cnt}, Ratio={ratio}:1")

            except subprocess.CalledProcessError as e:
                # 处理命令执行失败的情况 (例如 fse_testo1.exe 执行出错)
                print(f"❌ 命令执行失败，返回码 {e.returncode}")
                # 打印命令的标准错误输出，这通常包含错误信息
                print(f"错误输出:\n{e.stderr}")
            except FileNotFoundError:
                print("❌ 错误: fse_testo1.exe 未找到。请检查您的路径，或尝试使用 '.\\fse_testo1.exe'。")
            except subprocess.TimeoutExpired:
                print("❌ 错误: 命令执行超时。")
            except Exception as e:
                print(f"❌ 发生了一个意外错误: {e}")


# --- 实验结果总结并写入 CSV 文件 ---
print("\n" + "="*30)
print("实验结果总结")
print("="*30)

# 1. 格式化输出最终结果到控制台
print(f"{'Dataset':<20}{'o0':<10}{'o1':<10}{'o1_use_cnt':<15}{'Ratio':<10}")
print("-" * 65)
for res in results:
    print(f"{res['dataset']:<20}{res['o0']:<10.6g}{res['o1']:<10.6g}{str(res['o1_use_cnt']):<15}{res['ratio']:<10.2f}")

# 2. 写入 CSV 文件
csv_filepath = 'compression_test_results.csv'
fieldnames = ['dataset', 'o0', 'o1', 'o1_use_cnt', 'ratio']

try:
    with open(csv_filepath, 'w', newline='', encoding='utf-8') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        
        # 写入表头
        writer.writeheader()
        
        # 写入数据行
        writer.writerows(results)

    print(f"\n✅ 所有结果已成功保存到 CSV 文件: {csv_filepath}")

except Exception as e:
    print(f"\n❌ 写入 CSV 文件时发生错误: {e}")

# 5 4 3 2 1
# 3 2 1 