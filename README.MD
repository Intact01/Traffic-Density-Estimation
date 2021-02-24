# Traffic Density Estimation Using OpenCV

## Part 1 : Camera Angle Correction and cropping


### Installation and Building (for Ubuntu)

``` bash
$ sudo apt install libboost-all-dev  
$ make
```  

### Executing 
``` bash
$ ./main.out -f empty.jpg  
```  

### Clean Workspace
``` bash
$ make clean
```

### Allowed options:
> <table border="0">
 <tr>
    <td>-h, --help</td>
    <td>produce help message</td>
 </tr>
 <tr>
    <td>-f, --filename < filepath > </td>
    <td>image filename</td>
 </tr>
 <tr>
    <td>-c, --custom </td>
    <td>enter custom destination points</td>
 </tr>
</table>
