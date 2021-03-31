import subprocess
import matplotlib.pyplot as plt
import sys
import time
from os import path

file_path = path.dirname(path.abspath(__file__))
executable = 'bin/main'

time_list = []
utility_list = []
title = ""


def build():
    cmd = 'make'
    subprocess.call(cmd)


method = sys.argv[1]
print("method is", method)
if method == '1':
    x_axis_list = [i for i in range(1, 20)]
    args_list = [i for i in range(1, 20)]
    args = "-r"
    x_label = "frame_skips"
    title = "Method 1 - skipping frames"
elif method == '2':
    x_axis_list = [i/10 for i in range(3, 11)]
    args = "-x"
    args_list = [f"{int(1920*i)}x{int(1080*i)}" for i in x_axis_list]
    x_label = "scale"
    title = "Method 2 - changing resolution"
elif method == '4' or method == '3':
    args_list = [i for i in range(7, 1, -1)]
    x_axis_list = args_list + [0]
    args = '-t'
    x_label = "threads"
    if method == '3':
        title = "Method 3 - splitting spatially for threading"
    else:
        title = "Method 4 - splitting frame-wise for threading"


def calc(val, m, a):
    cmd = f"{executable} -m{m} {a}{val}"
    print(cmd)
    p = subprocess.Popen(
        cmd.split(), stdout=subprocess.PIPE, cwd=f"{file_path}/..")
    output, err = p.communicate()
    utility, execution_time = output.split()
    print(utility, execution_time)
    time_list.append(float(execution_time))
    utility_list.append(float(utility))


def plot_graph():
    for x_val in args_list:
        calc(x_val, method, args)
    if method == '3' or method == '4':
        calc("", 0, "")

    print(x_axis_list, time_list, utility_list)
    fig, ax = plt.subplots()
    ax.plot(x_axis_list, time_list, color="red", marker="o")
    ax.set_xlabel(x_label, fontsize=14)
    ax.set_ylabel("Runtime (in seconds)", color="red", fontsize=14)
    ax2 = ax.twinx()

    # make a plot with different y-axis using second axis object
    ax2.plot(x_axis_list,
             utility_list, color="blue", marker="o")
    ax2.set_ylabel("Utility (fraction)", color="blue", fontsize=14)
    plt.title(title)

    fig.savefig(f"{file_path}/graphs/Method{method}.png",
                format='png', dpi=100, bbox_inches='tight')


# build()
plot_graph()
