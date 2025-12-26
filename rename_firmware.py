import re
import os
import datetime
import shutil
from pathlib import Path

Import("env")

def rename_firmware_advanced(source, target, env):
    build_dir = Path(env.subst("$BUILD_DIR"))
    project_dir = Path(env.subst("$PROJECT_DIR"))
    firmware_name = env.subst("$PROGNAME")
    
    # 配置选项
    CONFIG = {
        'include_timestamp': True,  # 是否包含时间戳
        'timestamp_format': "%Y%m%d%H%M",  # 时间戳格式
        'create_clean_copy': True,  # 创建无时间戳的副本
        'keep_original': True,  # 保留原始 firmware.bin
        'output_dir': build_dir,  # 输出目录
        'prefix': "DRS",  # 文件名前缀
    }
    
    # 获取时间戳
    now = datetime.datetime.now()
    timestamp = now.strftime(CONFIG['timestamp_format'])
    date_str = now.strftime("%Y-%m-%d")
    time_str = now.strftime("%H:%M:%S")
    
    # 搜索版本信息
    fw_version, pcb_version = extract_version_info(project_dir)
    
    # 构建文件名
    base_name = f"{CONFIG['prefix']}_FW_{fw_version}_PCB_{pcb_version}"
    
    if CONFIG['include_timestamp']:
        final_name = f"{base_name}_{timestamp}.bin"
    else:
        final_name = f"{base_name}.bin"
    
    # 源文件路径
    src_path = build_dir / f"{firmware_name}.bin"
    
    if not src_path.exists():
        # 尝试其他可能的名称
        src_path = build_dir / "firmware.bin"
        if not src_path.exists():
            print("❌ 未找到固件文件")
            return
    
    # 目标文件路径
    dst_path = CONFIG['output_dir'] / final_name
    
    # 复制文件
    shutil.copy2(src_path, dst_path)
    
    # 创建无时间戳副本
    if CONFIG['create_clean_copy']:
        clean_name = f"{base_name}.bin"
        clean_path = CONFIG['output_dir'] / clean_name
        shutil.copy2(src_path, clean_path)
    
    # 删除原始文件（如果配置了）
    if not CONFIG['keep_original']:
        src_path.unlink()
    
    # 输出报告
    print_report(fw_version, pcb_version, date_str, time_str, final_name)

def extract_version_info(project_dir):
    """从项目中提取版本信息"""
    
    fw_version = "unknown"
    pcb_version = "unknown"
    
    # 递归搜索所有源文件
    for ext in ['*.h', '*.cpp', '*.ino', '*.c']:
        for file_path in project_dir.rglob(ext):
            try:
                content = file_path.read_text(encoding='utf-8', errors='ignore')
                
                # 提取固件版本
                if fw_version == "unknown":
                    fw_match = re.search(r'#define\s+LINE1\s+\(char\*\)"FW:\s*([\d\.]+)"', content)
                    if fw_match:
                        fw_version = fw_match.group(1).replace('.', '_')
                
                # 提取 PCB 版本
                if pcb_version == "unknown":
                    # 方法1: 直接找 PCB_VER
                    pcb_match = re.search(r'#define\s+PCB_VER\s+\(char\*\)"PCB:\s*([\d\.]+)"', content)
                    if pcb_match:
                        pcb_version = pcb_match.group(1).replace('.', '_')
                    else:
                        # 方法2: 查找启用的 V 定义
                        v_matches = re.findall(r'#define\s+(V\d+_\d+)', content)
                        for v_def in v_matches:
                            # 检查是否在 #ifdef 块中
                            pattern = rf'#ifdef\s+{v_def}.*?#define\s+{v_def}'
                            if re.search(pattern, content, re.DOTALL):
                                pcb_version = v_def.replace('V', '').replace('_', '_')
                                break
                
                if fw_version != "unknown" and pcb_version != "unknown":
                    return fw_version, pcb_version
                    
            except Exception as e:
                continue
    
    return fw_version, pcb_version

def print_report(fw_version, pcb_version, date_str, time_str, final_name):
    """打印漂亮的输出报告"""
    
    # 使用 ANSI 颜色代码
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    RESET = '\033[0m'
    
    print(f"\n{GREEN}{'='*60}{RESET}")
    print(f"{GREEN}🏗️  固件构建完成{RESET}")
    print(f"{GREEN}{'='*60}{RESET}")
    print(f"{YELLOW}📦 版本信息:{RESET}")
    print(f"   固件版本: {BLUE}{fw_version}{RESET}")
    print(f"   PCB 版本: {BLUE}{pcb_version}{RESET}")
    print(f"{YELLOW}📅 构建时间:{RESET}")
    print(f"   日期: {date_str}")
    print(f"   时间: {time_str}")
    print(f"{YELLOW}💾 生成文件:{RESET}")
    print(f"   {GREEN}{final_name}{RESET}")
    print(f"{GREEN}{'='*60}{RESET}\n")

# 注册构建后动作
env.AddPostAction("buildprog", rename_firmware_advanced)