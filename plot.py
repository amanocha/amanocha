from matplotlib import ticker
import matplotlib.pyplot as plt
import math
import numpy as np
import os
import sys

LABEL_FONTSIZE = 28
TICK_FONTSIZE = 24
ROUND = 1e7
outdir = "figs/"

app_names = []
num_tags = []
num_seqs = []

def parse(dir, name):
    global app_names, num_tags, num_seqs

    for app in os.listdir(dir):
        print(app)
        freqfile = open(dir + "/" + app + "/" + name + ".txt")
        freqdata = freqfile.readlines()
        entries = [line.replace('\n', '').split(' ') for line in freqdata]
        freqs = [int(entry[1]) for entry in entries]

        num_intervals = 10
        if (name == "addr" or name == "tags"):
            tags = [int(entry[0]) for entry in entries]
            x_interval = math.ceil(max(tags)/num_intervals/ROUND)*ROUND
            xticks = np.arange(0, x_interval*(num_intervals+1), x_interval)
            app_names.append(app)
            num_tags.append(len(tags))
            if (name == "addr"):
                xname = "Address"
            elif (name == "tags"):
                xname = "Tag"
        else:
            tags = [entry[0] for entry in entries]
            xticks = []
            num_seqs.append(len(tags))
            xname = "Tag Sequence ID"

        y_interval = math.ceil(max(freqs)/num_intervals/ROUND)*ROUND
        yticks = np.arange(0, y_interval*(num_intervals+1), y_interval)

        fig = plt.figure(figsize=(25.0, 15.0))
        fig.subplots_adjust(bottom=0.1)
        ax = fig.add_subplot(111)

        ax.scatter(tags, freqs, s=50)
        #ax.set_xticks(xticks)
        #ax.set_yticks(yticks)
        ax.set_xlabel(xname, size=LABEL_FONTSIZE)
        ax.set_ylabel("Frequency", size=LABEL_FONTSIZE)
        ax.set_title(app, size=1.25*LABEL_FONTSIZE)

        formatter = ticker.ScalarFormatter(useMathText=True)
        formatter.set_scientific(True)
        ax.xaxis.set_major_formatter(formatter)
        ax.set_yscale('log')
        plt.setp(ax.get_xticklabels(), fontsize=TICK_FONTSIZE)
        plt.setp(ax.get_yticklabels(), fontsize=TICK_FONTSIZE)

        #plt.show()
        plt.savefig(outdir + app + "_" + name + ".png")

def final_plot(name, y_data):
    fig = plt.figure(figsize=(25.0, 15.0))
    fig.subplots_adjust(bottom=0.1)
    ax = fig.add_subplot(111)

    x_pos = [i for i, _ in enumerate(app_names)]
    ax.bar(x_pos, y_data)
    ax.set_xticklabels(ax.get_xticks(), rotation = 90)
    ax.set_xlabel("App", size=LABEL_FONTSIZE)
    ax.set_ylabel("# Unique Tags", size=LABEL_FONTSIZE)
    plt.xticks(x_pos, app_names)
    plt.setp(ax.get_xticklabels(), fontsize=TICK_FONTSIZE)
    plt.setp(ax.get_yticklabels(), fontsize=TICK_FONTSIZE)

    plt.savefig(outdir + name + ".png")

def main():
    dir = sys.argv[1]
    parse(dir, "addr")
    parse(dir, "tags")
    sys.exit(1)

    final_plot("num_seqs", num_seqs)
    final_plot("num_tags", num_tags)

if __name__ == "__main__":
    main()
