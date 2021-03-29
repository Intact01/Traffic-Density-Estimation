import subprocess
import matplotlib.pyplot as plt
import sys
import time

executable = 'bin/main'


def build():
    cmd = 'make'
    subprocess.call(cmd)


method = sys.argv[1]
print("method is ", method)
if method == '1':
    x_axis_list = [i for i in range(1, 100)]
    args = "-r"
    x_label = "frame_skips"
elif method == '4' or method == '3':
    x_axis_list = [i for i in range(1, 10)]
    args = '-t'
    x_label = "threads"


def plot_graph():
    time_list = []
    utility_list = []
    for x_val in x_axis_list:
        # time.sleep(100)

        cmd = f"{executable} -m{method} {args}{x_val}"
        print(cmd)
        p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
        output, err = p.communicate()
        utility, execution_time = output.split()
        print(utility, execution_time)
        time_list.append(float(execution_time))
        utility_list.append(float(utility))

    fig, ax = plt.subplots()
    ax.plot(x_axis_list, time_list, color="red", marker="o")
    ax.set_xlabel(x_label, fontsize=14)
    ax.set_ylabel("Runtime (in seconds)", color="red", fontsize=14)
    ax2 = ax.twinx()
    # make a plot with different y-axis using second axis object
    ax2.plot(x_axis_list,
             utility_list, color="blue", marker="o")
    ax2.set_ylabel("Utility (fraction)", color="blue", fontsize=14)
    plt.show()

    # fig.savefig(f"Method{method}.jpg",
    #             format='jpeg',
    #             dpi=100,
    #             bbox_inches='tight')


# build()
plot_graph()
