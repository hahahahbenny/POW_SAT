import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import scienceplots
from matplotlib.font_manager import FontProperties
from mplfonts import use_font
from matplotlib.ticker import MultipleLocator
from matplotlib.ticker import FixedLocator
from matplotlib.ticker import FixedLocator, NullLocator

from matplotlib.scale import FuncScale
import numpy as np

def forward(y):
    return np.where(y <= 100, y / 100 * 0.7, 0.7 + (y - 100) / 3900 * 0.3)
def inverse(y_trans):
    return np.where(y_trans <= 0.7, y_trans / 0.7 * 100, 100 + (y_trans - 0.7) / 0.3 * 3900)

# crypto_withxor = [1.3,	1.77,	3.37,	5.95,	11.4,	25.34,	49.725,	99.105,	192.41,	491.04,	1143.07,	1755.42,	3690]
# crypto_withoutxor = [1.45,	1.48,	2.99,	5.41,	10.24,	20.49,	40.28,	79.64,	164.62,	370.72,	1042.08,	2069.55,	3186.7]
# y1 = [0.539 ,	1.255 ,	2.449 ,	3.656 ,	11.592 ,	21.406 ,	24.080 ,	8.315 ,	139.019 ,	152.201 ,	123.676 ,	335.616]
crypto_2018_1st             = [0.73, 	1.24, 	2.12, 	1.32, 	10.04, 	5.83, 	18.69, 	48.31, 	120.28]
kisssat_2024_1st            = [1.86,	2.48, 	3.63, 	2.32, 	13.40, 	14.46, 	33.05, 	63.53, 	36.18]
breakID_kissat_2024_2nd     = [2.65, 	2.73, 	4.15, 	4.81, 	4.85, 	15.91, 	25.13, 	52.44, 	111.36]
hkis_bva_2024_3rd           = [2.82, 	2.64, 	2.28, 	3.37, 	7.14, 	9.54, 	6.11, 	82.85, 	123.46]
hkis_PBVA_2024_3rd          = [2.37, 	3.07, 	3.41, 	2.77, 	5.04, 	12.40, 	25.91, 	27.64, 	87.02]
cadical_bva_2023_1st        = [3.06, 	3.32, 	3.36, 	6.89, 	6.74, 	6.85, 	6.87, 	51.76, 	109.98]
cadical_scavel_2023_2nd     = [1.27, 	1.58, 	1.62, 	5.17, 	5.41, 	5.53, 	5.45, 	53.58, 	108.39]
cadical_vivinst_3rd         = [1.24, 	1.50, 	1.50, 	4.72, 	4.73, 	4.81, 	4.75, 	47.47, 	98.52]
cadical_v2                  = [1.25, 	1.55, 	1.57, 	4.80, 	4.97, 	4.82, 	4.84, 	49.28, 	132.17]
x = np.arange(4, 13)

colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k', 'orange']

# 设置绘图风格
plt.style.use(['science', 'bright', 'no-latex','grid'])
plt.rcParams.update({
    'font.family': 'Source Han Sans',
    'axes.unicode_minus': False,
})
with plt.style.context(['science', 'ieee', 'grid', 'no-latex']):
    # plt.rcParams['font.family'] = 'NotoSerifCJK'
    # 创建图表
    fig, ax = plt.subplots(figsize=(4, 5), dpi=300)

    # 绘制数据线


    # 设置y轴范围
    
    # ax.set_yscale('function', functions=(forward, inverse))
    # 设置x轴和y轴的刻度
    ax.set_xlim(4, 13)
    ax.set_ylim(0, 135)
    ax.set_xticks(np.arange(4, 13, 1))


    ax.tick_params(axis='x', labelsize=12)  # x轴刻度字体为14
    ax.tick_params(axis='y', labelsize=12)  # y轴刻度字体为12

    # 修改刻度标签显示
    ax.set_ylabel('time(s)')
    ax.set_xlabel('nonce left bits')
    # y_tick = list(sorted([float(x) for x in set(kissat + crypto)]))
    
    # ax.set_ylim(min(y_tick), max(y_tick))
    # ax.set_yticks(y_tick)  
    # ax.yaxis.set_major_locator(FixedLocator(y_tick)) 
    ax.tick_params(which='minor', left=False)
# 设置标签（需与刻度位置一一对应）
    # ax.set_yticklabels([str(tick) for tick in y_tick]) 

    # ax.yaxis.set_minor_locator(NullLocator())
    ax.tick_params(which='minor', left=False)

    ax.plot(x, crypto_2018_1st, label="crypto_2018_1st", marker='o', markersize=3, color=colors[0])
    ax.plot(x, kisssat_2024_1st, label="kisssat_2024_1st", marker='o', markersize=3, color=colors[1])
    ax.plot(x, breakID_kissat_2024_2nd, label="breakID_kissat_2024_2nd", marker='o', markersize=3, color=colors[2])
    ax.plot(x, hkis_bva_2024_3rd, label="hkis_bva_2024_3rd", marker='o', markersize=3, color=colors[3])
    ax.plot(x, cadical_bva_2023_1st, label="cadical_bva_2023_1st", marker='o', markersize=3, color=colors[4])
    ax.plot(x, cadical_scavel_2023_2nd, label="cadical_scavel_2023_2nd", marker='o', markersize=3, color=colors[5])
    ax.plot(x, cadical_vivinst_3rd, label="cadical_vivinst_3rd", marker='o', markersize=3, color=colors[6])
    ax.plot(x, cadical_v2, label="cadical_v2", marker='o', markersize=3, color=colors[7])

    # ax.plot(x, crypto_withxor, label="withxor", marker='o', markersize=3, color=colors[0])
    # ax.plot(x, crypto_withoutxor, label="withoutxor", marker='o', markersize=3, color=colors[1])
    # 设置图例
    ax.legend(title="solver consume time")

    # 调整布局，确保不被裁切
    fig.tight_layout()

# 调整布局
plt.savefig('../result/image/2023_2024_sat_solver_cmp.png', dpi=300, bbox_inches='tight',facecolor='white')
# 显示图表
plt.show()
