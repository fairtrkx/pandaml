# PANDA Tracking Studies <!-- omit in toc -->

This repository contains the code for the following tasks:

1. **Simulate hits** in the PANDA tracking systems corresponding to different physical processes.
2. **Reconstruct particle tracks** from these hits using ideal track reconstruction.
3. **Export the hit (and ideal track) information** into .csv files so that they can be processed by a machine learning algorithm.
4. **Visualize** the different steps of the simulation chain in combination with the detector geometries.

## Table of Contents <!-- omit in toc -->

- [Dependencies](#dependencies)
- [Setup](#setup)
  - [1. Download the Repository Using Git](#1-download-the-repository-using-git)
  - [2. Add Modules to PandaRoot](#2-add-modules-to-pandaroot)
  - [3. Create \& Activate the Conda Environment](#3-create--activate-the-conda-environment)
  - [4. Set Bash Environment Variables](#4-set-bash-environment-variables)


## Dependencies

The following software is required to run the code in this repository:

- [Git](https://git-scm.com/downloads)
- [PandaRoot](https://git.panda.gsi.de/PandaRootGroup/PandaRoot) (currently using v13.1.0)
- [Anaconda](https://www.anaconda.com/download/success) / [Miniconda](https://docs.anaconda.com/free/miniconda/index.html)

## Setup

### 1. Download the Repository Using Git

```bash
git clone -b <branch> https://github.com/ai4fair/pandaml.git <path>/<name>
```

You should replace `<branch>` with the branch you want to use, e.g. `main`, `<path>` with the path to the repository, and `<name>` with the name you want to give to the folder where the repository is located.

### 2. Add Modules to PandaRoot

Copy the two PandaRoot modules [`PndMLTracker/`](PandaRootModules/PndMLTracker/) and [`PndTrackImport/`](PandaRootModules/PndTrackImport/) into the PandaRoot source directory. To do so execute the following commands while in the root directory of this repository:

```bash
cp -r PandaRootModules/PndMLTracker <path/to/PandaRoot/source>/tracking/
```

```bash
cp -r PandaRootModules/PndTrackImport/ <path/to/PandaRoot/source>/tracking/
```

Replace `<path/to/PandaRoot/source>` with the path to your PandaRoot source directory, where a `tracking` folder should exist.

Now change add the new modules to the `CMakeLists.txt` file which should be located at `<path/to/PandaRoot/source>`. To do so open `CMakeLists.txt` with you preferred editor, e.g.

```bash
code <path/to/PandaRoot/source>/CMakeLists.txt
```

and then add the lines

```cmake
add_subdirectory (tracking/PndMLTracker)
add_subdirectory (tracking/PndTrackImport)
```

at the part of the code, where all other subdirectories are added.

Then recompile PandaRoot by making a `build` directory in your PandaRoot path `<path/to/PandaRoot>`

```bash
mkdir <path/to/PandaRoot>/build
```

and change into the `build` directory

```bash
cd <path/to/PandaRoot>/build
```

then prepare the compilation with CMake

```bash
cmake <OTHER_FLAGS> -DCMAKE_INSTALL_PREFIX=../install ../<source>
```

Here `<OTHER_FLAGS>` is a place holder for flags that you might want to use for the compilation, e.g. `-DBUILD_PANDA_WITHOUT_CUDA` to build PandaRoot without cuda, but it can also be left empty. `<source>` should be replaced with the name of the PandaRoot source directory that you downloaded from git.

### 3. Create & Activate the Conda Environment

To use the jupyter notebooks to plot the simulation results, the corresponding conda environment has to be created and activated. To do so, go to the [`condaEnvironments/`](condaEnvironments/) folder in the repository

```bash
cd condaEnvironments/
```

create the conda environment

```bash
conda env create --file visualization.yml
```

and activate the environment

```bash
conda activate vis
```

### 4. Set Bash Environment Variables

Adjust and add the following paths to your bash environment e.g. your `.bashrc` most commonly located at you home directory.

```bash
export SIM_FILE_PATH=<output/path/to/your/simulation/files>
```

Path to the directory where you want to save the results of the simulation chain (`.root` and `.log` files).

```bash
export ML_FILE_PATH=<output/path/to/your/ML/files>
```

Path to the directory where the `.cvs` files are stored for the machine learning algorithm.

```bash
export VIS_PATH=<output/path/to/your/plots/and/images>
```

Path to the directory where the plots and images created in the visualization step are saved.