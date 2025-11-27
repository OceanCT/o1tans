import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import io
import os
import shutil

# --- 1. Raw Experimental Data (CSV format) ---
RAW_DATA = """dataset,o0,o1,o1_use_cnt,ratio
dickens,0,0,96,2.25
dickens,0,0.1,96,2.0
dickens,0,0.2,96,1.9
dickens,0,1.0,96,1.79
dickens,0.00390625,0,29,2.22
dickens,0.00390625,0.1,29,1.98
dickens,0.00390625,0.2,29,1.89
dickens,0.00390625,1.0,29,1.79
dickens,0.015625,0,20,2.15
dickens,0.015625,0.1,20,1.95
dickens,0.015625,0.2,20,1.87
dickens,0.015625,1.0,20,1.79
dickens,0.5,0,0,1.75
dickens,0.5,0.1,0,1.75
dickens,0.5,0.2,0,1.75
dickens,0.5,1.0,0,1.75
dickens,1.0,0,0,1.75
dickens,1.0,0.1,0,1.75
dickens,1.0,0.2,0,1.75
dickens,1.0,1.0,0,1.75
mozilla,0,0,256,1.55
mozilla,0,0.1,256,1.43
mozilla,0,0.2,256,1.41
mozilla,0,1.0,256,1.28
mozilla,0.00390625,0,53,1.45
mozilla,0.00390625,0.1,53,1.41
mozilla,0.00390625,0.2,53,1.39
mozilla,0.00390625,1.0,53,1.28
mozilla,0.015625,0,3,1.27
mozilla,0.015625,0.1,3,1.31
mozilla,0.015625,0.2,3,1.31
mozilla,0.015625,1.0,3,1.28
mozilla,0.5,0,0,1.28
mozilla,0.5,0.1,0,1.28
mozilla,0.5,0.2,0,1.28
mozilla,0.5,1.0,0,1.28
mozilla,1.0,0,0,1.28
mozilla,1.0,0.1,0,1.28
mozilla,1.0,0.2,0,1.28
mozilla,1.0,1.0,0,1.28
mr,0,0,256,2.01
mr,0,0.1,256,2.24
mr,0,0.2,256,2.22
mr,0,1.0,256,2.15
mr,0.00390625,0,18,1.96
mr,0.00390625,0.1,18,2.15
mr,0.00390625,0.2,18,2.13
mr,0.00390625,1.0,18,2.15
mr,0.015625,0,5,1.91
mr,0.015625,0.1,5,2.11
mr,0.015625,0.2,5,2.1
mr,0.015625,1.0,5,2.15
mr,0.5,0,1,1.86
mr,0.5,0.1,1,2.09
mr,0.5,0.2,1,2.09
mr,0.5,1.0,1,2.15
mr,1.0,0,0,2.15
mr,1.0,0.1,0,2.15
mr,1.0,0.2,0,2.15
mr,1.0,1.0,0,2.15
nci,0,0,62,2.49
nci,0,0.1,62,2.31
nci,0,0.2,62,2.31
nci,0,1.0,62,3.28
nci,0.00390625,0,16,2.41
nci,0.00390625,0.1,16,2.25
nci,0.00390625,0.2,16,2.26
nci,0.00390625,1.0,16,3.27
nci,0.015625,0,6,2.34
nci,0.015625,0.1,6,2.22
nci,0.015625,0.2,6,2.24
nci,0.015625,1.0,6,3.27
nci,0.5,0,1,2.41
nci,0.5,0.1,1,2.36
nci,0.5,0.2,1,2.36
nci,0.5,1.0,1,3.27
nci,1.0,0,0,3.27
nci,1.0,0.1,0,3.27
nci,1.0,0.2,0,3.27
nci,1.0,1.0,0,3.27
ooffice,0,0,256,1.51
ooffice,0,0.1,256,1.35
ooffice,0,0.2,256,1.33
ooffice,0,1.0,256,1.2
ooffice,0.00390625,0,56,1.45
ooffice,0.00390625,0.1,56,1.32
ooffice,0.00390625,0.2,56,1.3
ooffice,0.00390625,1.0,56,1.2
ooffice,0.015625,0,9,1.31
ooffice,0.015625,0.1,9,1.26
ooffice,0.015625,0.2,9,1.25
ooffice,0.015625,1.0,9,1.2
ooffice,0.5,0,0,1.2
ooffice,0.5,0.1,0,1.2
ooffice,0.5,0.2,0,1.2
ooffice,0.5,1.0,0,1.2
ooffice,1.0,0,0,1.2
ooffice,1.0,0.1,0,1.2
ooffice,1.0,0.2,0,1.2
ooffice,1.0,1.0,0,1.2
osdb,0,0,256,1.51
osdb,0,0.1,256,1.35
osdb,0,0.2,256,1.32
osdb,0,1.0,256,1.2
osdb,0.00390625,0,76,1.43
osdb,0.00390625,0.1,76,1.28
osdb,0.00390625,0.2,76,1.27
osdb,0.00390625,1.0,76,1.2
osdb,0.015625,0,13,1.29
osdb,0.015625,0.1,13,1.24
osdb,0.015625,0.2,13,1.23
osdb,0.015625,1.0,13,1.2
osdb,0.5,0,0,1.2
osdb,0.5,0.1,0,1.2
osdb,0.5,0.2,0,1.2
osdb,0.5,1.0,0,1.2
osdb,1.0,0,0,1.2
osdb,1.0,0.1,0,1.2
osdb,1.0,0.2,0,1.2
osdb,1.0,1.0,0,1.2
reymont,0,0,256,2.7
reymont,0,0.1,256,2.22
reymont,0,0.2,256,2.05
reymont,0,1.0,256,1.6
reymont,0.00390625,0,40,2.72
reymont,0.00390625,0.1,40,2.17
reymont,0.00390625,0.2,40,2.02
reymont,0.00390625,1.0,40,1.6
reymont,0.015625,0,17,2.31
reymont,0.015625,0.1,17,1.97
reymont,0.015625,0.2,17,1.87
reymont,0.015625,1.0,17,1.6
reymont,0.5,0,0,1.6
reymont,0.5,0.1,0,1.6
reymont,0.5,0.2,0,1.6
reymont,0.5,1.0,0,1.6
reymont,1.0,0,0,1.6
reymont,1.0,0.1,0,1.6
reymont,1.0,0.2,0,1.6
reymont,1.0,1.0,0,1.6
samba,0,0,256,1.82
samba,0,0.1,256,1.57
samba,0,0.2,256,1.47
samba,0,1.0,256,1.3
samba,0.00390625,0,60,1.76
samba,0.00390625,0.1,60,1.55
samba,0.00390625,0.2,60,1.46
samba,0.00390625,1.0,60,1.3
samba,0.015625,0,17,1.52
samba,0.015625,0.1,17,1.44
samba,0.015625,0.2,17,1.39
samba,0.015625,1.0,17,1.3
samba,0.5,0,0,1.3
samba,0.5,0.1,0,1.3
samba,0.5,0.2,0,1.3
samba,0.5,1.0,0,1.3
samba,1.0,0,0,1.3
samba,1.0,0.1,0,1.3
samba,1.0,0.2,0,1.3
samba,1.0,1.0,0,1.3
sao,0,0,256,1.33
sao,0,0.1,256,1.25
sao,0,0.2,256,1.19
sao,0,1.0,256,1.06
sao,0.00390625,0,65,1.27
sao,0.00390625,0.1,65,1.21
sao,0.00390625,0.2,65,1.17
sao,0.00390625,1.0,65,1.06
sao,0.015625,0,6,1.1
sao,0.015625,0.1,6,1.08
sao,0.015625,0.2,6,1.06
sao,0.015625,1.0,6,1.06
sao,0.5,0,0,1.06
sao,0.5,0.1,0,1.06
sao,0.5,0.2,0,1.06
sao,0.5,1.0,0,1.06
sao,1.0,0,0,1.06
sao,1.0,0.1,0,1.06
sao,1.0,0.2,0,1.06
sao,1.0,1.0,0,1.06
webster,0,0,97,2.32
webster,0,0.1,97,1.98
webster,0,0.2,97,1.86
webster,0,1.0,97,1.64
webster,0.00390625,0,33,2.23
webster,0.00390625,0.1,33,1.93
webster,0.00390625,0.2,33,1.83
webster,0.00390625,1.0,33,1.64
webster,0.015625,0,22,2.12
webster,0.015625,0.1,22,1.87
webster,0.015625,0.2,22,1.78
webster,0.015625,1.0,22,1.64
webster,0.5,0,0,1.6
webster,0.5,0.1,0,1.6
webster,0.5,0.2,0,1.6
webster,0.5,1.0,0,1.6
webster,1.0,0,0,1.6
webster,1.0,0.1,0,1.6
webster,1.0,0.2,0,1.6
webster,1.0,1.0,0,1.6
x-ray,0,0,256,1.4
x-ray,0,0.1,256,1.26
x-ray,0,0.2,256,1.25
x-ray,0,1.0,256,1.21
x-ray,0.00390625,0,14,1.28
x-ray,0.00390625,0.1,14,1.21
x-ray,0.00390625,0.2,14,1.21
x-ray,0.00390625,1.0,14,1.21
x-ray,0.015625,0,12,1.28
x-ray,0.015625,0.1,12,1.21
x-ray,0.015625,0.2,12,1.21
x-ray,0.015625,1.0,12,1.21
x-ray,0.5,0,0,1.21
x-ray,0.5,0.1,0,1.21
x-ray,0.5,0.2,0,1.21
x-ray,0.5,1.0,0,1.21
x-ray,1.0,0,0,1.21
x-ray,1.0,0.1,0,1.21
x-ray,1.0,0.2,0,1.21
x-ray,1.0,1.0,0,1.21
xml,0,0,99,2.23
xml,0,0.1,99,1.81
xml,0,0.2,99,1.67
xml,0,1.0,99,1.44
xml,0.00390625,0,53,2.17
xml,0.00390625,0.1,53,1.78
xml,0.00390625,0.2,53,1.65
xml,0.00390625,1.0,53,1.44
xml,0.015625,0,20,1.83
xml,0.015625,0.1,20,1.62
xml,0.015625,0.2,20,1.55
xml,0.015625,1.0,20,1.44
xml,0.5,0,0,1.44
xml,0.5,0.1,0,1.44
xml,0.5,0.2,0,1.44
xml,0.5,1.0,0,1.44
xml,1.0,0,0,1.44
xml,1.0,0.1,0,1.44
xml,1.0,0.2,0,1.44
xml,1.0,1.0,0,1.44
"""

OUTPUT_DIR = './pics'
MAIN_FIGURE_FILENAME = 'all_datasets_compression_ratios.png'

def plot_compression_results(df):
    """
    Plots grouped bar charts for each dataset showing Compression Ratio vs O0, 
    grouped by O1 parameter. Saves each subplot and the overall figure.
    """
    
    # --- 1. Font and Style Settings (Times New Roman style) ---
    plt.rcParams['font.family'] = 'serif'
    # Prioritize Times New Roman, fall back to other serif fonts
    plt.rcParams['font.serif'] = ['Times New Roman', 'DejaVu Serif', 'Computer Modern Roman']
    plt.rcParams['font.size'] = 10 
    plt.style.use('ggplot') 

    # --- 2. Directory Setup ---
    # Clear and recreate the directory
    if os.path.exists(OUTPUT_DIR):
        shutil.rmtree(OUTPUT_DIR)
    os.makedirs(OUTPUT_DIR)
    print(f"Created output directory: {OUTPUT_DIR}")

    datasets = df['dataset'].unique()
    o1_values = sorted(df['o1'].unique())
    n_o1 = len(o1_values)
    
    # Create subplot layout based on the number of datasets
    num_cols = 3
    num_rows = len(datasets) // num_cols + (len(datasets) % num_cols > 0)
    
    # Create the main figure
    fig, axes = plt.subplots(
        nrows=num_rows, 
        ncols=num_cols, 
        figsize=(18, 5 * num_rows)
    )
    # Flatten axes array for easy iteration
    axes = axes.flatten() if isinstance(axes, np.ndarray) and axes.ndim > 1 else np.array([axes]).flatten()

    for i, dataset in enumerate(datasets):
        # 1. Prepare data for the current dataset
        data = df[df['dataset'] == dataset].copy()
        
        # Pivot: Index=o0 (X-axis), Columns=o1 (Groups), Values=ratio (Y-axis)
        pivot_df = data.pivot(index='o0', columns='o1', values='ratio')
        pivot_df = pivot_df.sort_index(ascending=True) # Sort by o0
        
        # Get O0 to O1_USE_CNT mapping for X-tick labels
        o0_to_o1_cnt = data.drop_duplicates(subset=['o0', 'o1_use_cnt']).set_index('o0')['o1_use_cnt']
        
        # 2. Define plotting parameters
        x_values = pivot_df.index
        x_positions = np.arange(len(x_values))
        bar_width = 0.8 / n_o1
        
        # --- 3. Plot on the main figure's subplot ---
        ax_main = axes[i]
        
        for j, o1 in enumerate(o1_values):
            # Calculate offset to center the group of bars
            offset = (j - (n_o1 - 1) / 2) * bar_width
            
            bars = ax_main.bar(
                x_positions + offset, 
                pivot_df[o1].fillna(0),
                width=bar_width, 
                label=f'$O_1$={o1}', # Use LaTeX for O1
                alpha=0.8,
                edgecolor='white'
            )
            
            # Add value labels on top of the bars
            for bar in bars:
                height = bar.get_height()
                if height > 0:
                    ax_main.text(
                        bar.get_x() + bar.get_width() / 2., 
                        height + 0.01,
                        f'{height:.2f}',
                        ha='center', 
                        va='bottom',
                        fontsize=7,
                        color='black'
                    )

        # 4. Set Main Subplot Labels and Title (English)
        ax_main.set_title(f'Dataset: {dataset}', fontsize=12)
        ax_main.set_ylabel('Compression Ratio', fontsize=10)
        
        # Set X-axis tick labels combining O0 and O1_USE_CNT
        tick_labels = [
            f"$O_0$={o0:.6g}\n(Cnt={o0_to_o1_cnt.loc[o0]})" 
            for o0 in x_values
        ]
        
        ax_main.set_xticks(x_positions)
        ax_main.set_xticklabels(tick_labels, rotation=45, ha='right', fontsize=9)
        ax_main.set_xlabel('$O_0$ Parameter ($O_{1\_USE\_CNT}$)', fontsize=10) # Use LaTeX for O0, O1_USE_CNT
        
        ax_main.legend(title='$O_1$ Parameter', fontsize=8, loc='best')
        ax_main.grid(axis='y', linestyle='--', alpha=0.5)
        ax_main.set_axisbelow(True)


        # --- 5. Save Each Subplot Individually ---
        fig_single, ax_single = plt.subplots(figsize=(8, 6))

        # Redraw to the separate figure
        for j, o1 in enumerate(o1_values):
            offset = (j - (n_o1 - 1) / 2) * bar_width
            bars = ax_single.bar(
                x_positions + offset, 
                pivot_df[o1].fillna(0),
                width=bar_width, 
                label=f'$O_1$={o1}',
                alpha=0.8,
                edgecolor='white'
            )
            
            for bar in bars:
                height = bar.get_height()
                if height > 0:
                    ax_single.text(
                        bar.get_x() + bar.get_width() / 2., 
                        height + 0.01,
                        f'{height:.2f}',
                        ha='center', 
                        va='bottom',
                        fontsize=8,
                        color='black'
                    )
                    
        # Set labels for the individual figure
        ax_single.set_title(f'Dataset: {dataset}', fontsize=14)
        ax_single.set_ylabel('Compression Ratio', fontsize=12)
        ax_single.set_xticks(x_positions)
        ax_single.set_xticklabels(tick_labels, rotation=45, ha='right', fontsize=10)
        ax_single.set_xlabel('$O_0$ Parameter ($O_{1\_USE\_CNT}$)', fontsize=12)
        ax_single.legend(title='$O_1$ Parameter', fontsize=10, loc='best')
        ax_single.grid(axis='y', linestyle='--', alpha=0.5)
        ax_single.set_axisbelow(True)
        plt.tight_layout()
        
        # Save the single subplot file
        single_filename = f'{dataset}_compression_ratio.png'
        fig_single.savefig(os.path.join(OUTPUT_DIR, single_filename), dpi=300)
        plt.close(fig_single) # Close the single figure

    # Remove unused subplots from the main figure
    for j in range(len(datasets), len(axes)):
        fig.delaxes(axes[j])
        
    plt.tight_layout()
    
    # --- 6. Save the Overall Figure ---
    fig.savefig(os.path.join(OUTPUT_DIR, MAIN_FIGURE_FILENAME), dpi=300)
    print(f"Saved main figure to {os.path.join(OUTPUT_DIR, MAIN_FIGURE_FILENAME)}")
    
    # Optional: Display the main figure
    plt.show()

# --- Execute plotting ---
try:
    df = pd.read_csv(io.StringIO(RAW_DATA), dtype={'o0': float, 'o1': float, 'o1_use_cnt': int, 'ratio': float})
    plot_compression_results(df)
    print("\nAll charts have been generated and saved to the './pics/' directory as requested.")
except Exception as e:
    # Print the error for debugging purposes
    print(f"An error occurred during data processing or plotting: {e}")