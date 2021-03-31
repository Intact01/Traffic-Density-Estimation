# Traffic Density Estimation Using OpenCV

## Part 3 : Analysis of Runtime-Utility Trade-offs


### Installation and Building (for Ubuntu 20.04 )

``` bash
$ sudo apt install libboost-program-options-dev
$ sudo apt install python3.8 python3.8-dev python3-matplotlib python3-numpy
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
>| -f, --file < filepath >    | video filepath      |
>| -s, --save < filepath >    | path to image of saved graph,  *Default : output.png*   |
>| -m, --method< method >      | choose method to apply |
>| -t, --thread < num_thread >  | number of threads (for method 3,4), *Default: 1* |
>| -r, --frameskip < frameskip >    | to process every rth frame (for method 1), *Default : 1*      |
>| -v, --verbose    | enable logging |
<br/>

### Libraries Used :
- **OpenCV** - For image and video operations and plotting graph in realtime 
- **Boost** - For argument parser
- **matplotlib** - For plotting final graph 
- **matplotlib-cpp** - C++ bindings for matplotlib [link](https://github.com/lava/matplotlib-cpp)



