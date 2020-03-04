## 🎓 Hideproc V2

The implementation of the external kernel module called hideproc

## ✨ Features

- Ability to hide the process from the machine's view
- Hiding process time will be calculated for each hide
- Verbosing
- Response when the hiding process is done or failed

## 🚀 Getting Started

1. **Compliing first.**

The c file should be complied first in order to get the .ko file

2. **Initialize the kernel modules.**

run the module with specific params 

```sh
sudo insmod hideproc.ko verb=[1 for enable verbose] target_pid=[process_id to hide]
```
Example
```sh
sudo insmod hideproc.ko verb=1 target_pid=2672
```

3. **Proof.**
you can check if the process is already hided or not

```sh
ps aux | grep [process_name]
```
Example
```sh
ps aux | grep putfile
```


## 🌟 Preview

<img src="https://www.img.in.th/images/aca5ed664d921f49cc831ed254a419bf.png" border="0" />

<p align="center">
  <b>: Contact me By :</b><br>
  <a href="https://www.facebook.com/thiti.developer">Facebook</a> |
  <a href="https://www.instagram.com/thiti.mwk/">Instagram</a> |
  <a href="https://www.linkedin.com/in/thiti-mahawannakit-558791183/">LinkedIn</a>
  <br><br>
  <img src="https://media.giphy.com/media/h1u6yvxlVKmfLiSryA/giphy.gif" width="250" height="220">
</p>

