from matplotlib import ticker
import matplotlib.pyplot as plt
import math
import numpy as np
import os
import sys

LABEL_FONTSIZE = 28
TICK_FONTSIZE = 24
ROUND = 1e7
size=(25.0, 15.0)
color='blue'
outdir = ""

app_names = []
num_addr = []
num_tags = []
num_pcs = []
num_addr_seqs = []
num_tag_seqs = []
num_tag_pcs = []
num_addr_combos = []
num_tag_combos = []
avg_per_tag = []
avg_per_pc = []

def parse(dir, name):
    global app_names, num_addr, num_tags, num_pcs, num_addr_seqs, num_tag_seqs, num_tag_pcs, num_addr_combos, num_tag_combos, avg_per_tag, avg_per_pc

    for app in sorted(os.listdir(dir)):

        # ----- PARSE DATA -----
        print(app)
        freqfile = open(dir + "/" + app + "/" + name + ".txt")
        freqdata = freqfile.readlines()
        entries = [line.replace('\n', '').split(' ') for line in freqdata]
        freqs = [int(entry[1]) for entry in entries]

        num_intervals = 10
        if (name == "addr" or name == "tags" or name == "pcs"):
            tags = [int(entry[0]) for entry in entries]
            #x_interval = math.ceil(max(tags)/num_intervals/ROUND)*ROUND
            #xticks = np.arange(0, x_interval*(num_intervals+1), x_interval)
            if (name == "addr"):
                app_names.append(app)
                xname = "Address"
                num_addr.append(len(tags))
            elif (name == "tags"):
                xname = "Tag"
                num_tags.append(len(tags))
            elif (name == "pcs"):
                xname = "PC"
                num_pcs.append(len(tags))
        else:
            tags = np.arange(len(freqs))
            xticks = []
            if (name == "addr_seqs"):
                addr_lists = [entry[0].split("_") for entry in entries]
                combos = {}
                for e in range(len(addr_lists)):
                  freq = freqs[e]
                  entry = addr_lists[e]
                  combo = frozenset(entry)
                  if combo not in combos:
                    combos[combo] = 0
                  combos[combo] += freq
                freqs = combos.values()
                tags = np.arange(len(freqs))
                xname = "Address Combination ID"
                num_addr_combos.append(len(tags))
 
                #xname = "Address Sequence ID"
                #num_addr_seqs.append(len(tags))
            elif (name == "tag_seqs"):
                tag_lists = [entry[0].split("_") for entry in entries]
                combos = {}
                for e in range(len(tag_lists)):
                  freq = freqs[e]
                  entry = tag_lists[e]
                  combo = frozenset(entry)
                  if combo not in combos:
                    combos[combo] = 0
                  combos[combo] += freq
                freqs = combos.values()
                tags = np.arange(len(freqs))
                xname = "Tag Combination ID"
                num_tag_combos.append(len(tags))
                
                #xname = "Tag Sequence ID"
                #num_tag_seqs.append(len(tags))
            elif (name == "tag_pc"):
                app_names.append(app)
                lists = [entry[0].split("_") for entry in entries]
                tags = {}
                pcs = {}
                for e in range(len(lists)):
                  freq = freqs[e]
                  tag = int(lists[e][0])
                  pc = int(lists[e][1])
                  if tag not in tags:
                    tags[tag] = []
                  if pc not in tags[tag]:
                    tags[tag].append(pc)
                  if pc not in pcs:
                    pcs[pc] = []
                  if tag not in pcs[pc]:
                    pcs[pc].append(tag)

                tag_sizes = tags.values()
                freqs1 = [len(sz) for sz in tag_sizes]
                avg1 = float(sum(freqs1))/len(freqs1)
                avg_per_tag.append(avg1)
                tags1 = tags.keys()
                plot(tags1, freqs1, "Tag", app, name + "_tag")

                pc_sizes = pcs.values()
                freqs2 = [len(sz) for sz in pc_sizes]
                avg2 = float(sum(freqs2))/len(freqs2)
                avg_per_pc.append(avg2)
                tags2 = pcs.keys() 
                plot(tags2, freqs2, "PC", app, name + "_pc")

                #xname = "Tag-PC Sequence ID"
                #num_tag_pcs.append(len(tags))

        #y_interval = math.ceil(max(freqs)/num_intervals/ROUND)*ROUND
        #yticks = np.arange(0, y_interval*(num_intervals+1), y_interval)
        #plot(tags, freqs, xname, app, name)

def plot(x, y, xname, app, name):
    # ----- CREATE FREQ PLOT -----
    fig = plt.figure(figsize=size)
    fig.subplots_adjust(bottom=0.1)
    ax = fig.add_subplot(111)

    ax.scatter(x, y, s=50)
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
    plt.savefig(outdir + app + "_" + name + "_combos.png", bbox_inches='tight')

    '''
    # ----- CREATE HISTOGRAM -----
    fig = plt.figure(figsize=size)
    fig.subplots_adjust(bottom=0.1)
    ax = fig.add_subplot(111)

    ax.hist(y, bins='auto', color=color)
    ax.set_xlabel("Bin", size=LABEL_FONTSIZE)
    ax.set_ylabel("Frequency", size=LABEL_FONTSIZE)
    ax.set_title(app, size=1.25*LABEL_FONTSIZE)
    ax.set_yscale('log')
    plt.setp(ax.get_xticklabels(), fontsize=TICK_FONTSIZE)
    plt.setp(ax.get_yticklabels(), fontsize=TICK_FONTSIZE)
        
    #plt.show()
    plt.savefig(outdir + app + "_" + name + "_hist.png", bbox_inches='tight')
    '''

def final_plot(name, y_data):
    fig = plt.figure(figsize=(25.0, 15.0))
    fig.subplots_adjust(bottom=0.1)
    ax = fig.add_subplot(111)

    x_pos = [i for i, _ in enumerate(app_names)]
    ax.bar(x_pos, y_data)
    ax.set_xticklabels(ax.get_xticks(), rotation = 90)
    ax.set_xlabel("App", size=LABEL_FONTSIZE)
    ax.set_ylabel("# Unique " + name, size=LABEL_FONTSIZE)
    plt.xticks(x_pos, app_names)
    plt.setp(ax.get_xticklabels(), fontsize=TICK_FONTSIZE)
    plt.setp(ax.get_yticklabels(), fontsize=TICK_FONTSIZE)

    plt.savefig(outdir + name.lower() + ".png")

def main():
    global outdir

    dir = sys.argv[1]
    outdir = dir.replace("output", "figs")
    
    '''
    parse(dir, "addr")
    final_plot("Addresses", num_addr)
    parse(dir, "tags")
    final_plot("Tags", num_tags)
    parse(dir, "pcs")
    final_plot("PCs", num_pcs)
    '''

    #parse(dir, "addr_seqs")
    #parse(dir, "tag_seqs")
    parse(dir, "tag_pc")

    #final_plot("Address Sequences", num_addr_seqs)
    #final_plot("Tag Sequences", num_tag_seqs)
    #final_plot("Tag-PC Sequences", num_tag_pcs)
    
    #final_plot("2-Address Combinations", num_addr_combos)
    #final_plot("2-Tag Combinations", num_tag_combos)
    final_plot("Avg # PCs Per Tag", avg_per_tag)
    final_plot("Avg # Tags Per PC", avg_per_pc)

if __name__ == "__main__":
    main()
