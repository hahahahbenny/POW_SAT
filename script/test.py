import numpy as np
from matplotlib.ticker import FixedLocator, NullLocator
import matplotlib.pyplot as plt

def forward(y):
    return np.where(y <= 200, y / 200 * 0.7, 0.7 + (y - 200) / 3800 * 0.3)
def inverse(y_trans):
    return np.where(y_trans <= 0.7, y_trans / 0.7 * 200, 200 + (y_trans - 0.7) / 0.3 * 3800)

withoutxor = [1.38,	1.435, 2.875, 5.08, 9.835, 19.725, 38.72, 77.05, 157.725, 351.35, 983.96, 1951.15, 2986.08]
withxor = [1.3, 1.77, 3.37,	5.95, 11.4,	25.34, 49.725, 99.105, 192.41, 491.04, 1143.07, 1755.42, 3779.08]
x = np.arange(4, 17)


# 数据预处理
y_tick = sorted({float(x) for x in set(withoutxor + withxor)})  # 确保无异常值

# 创建图表
fig, ax = plt.subplots()

# 设置非线性缩放（需验证forward/inverse函数）
ax.set_yscale('function', functions=(forward, inverse))

# 关键修正步骤
ax.set_xlim(4, 16)
ax.set_xticks(np.arange(4, 17, 1))
ax.set_ylim(min(y_tick), max(y_tick))  # 显式约束范围

ax.plot(x, withxor, label="withxor", marker='o', markersize=3)
ax.plot(x, withoutxor, label="withoutxor", marker='o', markersize=3)


# 设置主刻度
ax.set_yticks(y_tick)
ax.set_yticklabels([str(tick) for tick in y_tick])

# 强制禁用次要刻度（双重保险）
ax.yaxis.set_minor_locator(NullLocator())
ax.tick_params(which='minor', left=False)

# 标签设置
ax.set_ylabel('time(s)')
ax.set_xlabel('nonce left bits')

# 优化布局
fig.tight_layout()

plt.savefig('./start_cur_cmp.png', dpi=300, bbox_inches='tight',facecolor='white')
plt.show()