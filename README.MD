# Traffic Density Estimation Using OpenCV

Analysis of runtime-utility trade offs for different methods to estimate the dynamic and static traffic density using OpenCV libraries.  

### Installation and Building (for Ubuntu 20.04 )

``` bash
$ sudo apt install libboost-program-options-dev
$ sudo apt install python3.8 python3.8-dev python3-matplotlib python3-numpy
$ mkdir -p bin
$ make
```  
*** For Older versions of Ubuntu, replace with appropriate versions of python3 in installation and in makefile

### Executing 
``` bash
$ bin/main -f trafficvideo.mp4 -m0
```  
More execution options are given below.
<br/>
<br/>

### Clean Workspace
``` bash
$ make clean
```

<br/>

### Allowed options:
>| Flag      | Usage           |
>| ------------- |:-------------:|
>| -h, --help                     | produce help message |
>| -f, --file < filepath >    | video filepath, *Default : input/trafficvideo.mp4*      |
>| -s, --save < filepath >    | path to image of saved graph,  *Default : output/output.png*   |
>| -m, --method< method >      | choose method to apply,  *Default : 0* |
>| -t, --thread < num_thread >  | number of threads (for method 3,4), *Default: 4* |
>| -r, --frameskip < frameskip >    | to process every rth frame (for method 1), *Default : 1*      |
>| -v, --verbose    | enable logging |
>| -x, --resolution  | provide resolution at which to process image (for method 2)
<br/>

### Argument --method
- *-m0* - Normal(baseline) method for queue density calculation
- *-m1* - Subsampling (frameskip) method for queue density calculation ( Method 1)
- *-m2* - Reduce resolution method for queue density calculation (Method 2)
- *-m3* - Dividing work spatially for queue density calculation (Method 3)
- *-m4* - Dividing work frame-wise for queue density calculation (Method 4)
- *-m5* - Sparse optical flow method for moving density calculation (Method 5)
- *-m6* - Dense optical flow method (baseline) for moving density calculation
<br>

### Folders' description
- *src* - Contains all the cpp and header files
- *input* - Default location for input video file
- *output* - Contains baseline data (density.csv) and is the default output location for the output graph of traffic density
- *bin* - Contains executable

<br>
### Libraries Used :
- **OpenCV** - For image and video operations and plotting graph in realtime 
- **Boost** - For argument parser
- **matplotlib** - For plotting final graph 
- **matplotlib-cpp** - C++ bindings for matplotlib [link](https://github.com/lava/matplotlib-cpp)



