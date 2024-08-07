# Port TinyMaix to Nuclei RISC-V Processor

Nuclei System Technology is a RISC-V CPU IP processor vendor, click https://nucleisys.com/ to learn more.

Nuclei SDK is an open source software project to support Nuclei RISC-V CPU embedded software development,
support Nuclei 200/300/600/900 series processor, and integrated into Nuclei Studio IDE via NPK package.

TinyMaix port to Nuclei RISC-V Processor can be found in https://github.com/Nuclei-Software/npk-tinymaix

## Test Nuclei RISC-V Processor

| Item         | Parameter      |
| ------------ | -------------- |
| CPU Core     | Nuclei 200/300/600/900 series |
| Arch         | RV32/RV64      |
| Freq         | 16MHz/100MHz   |
| Flash        | 16MBytes       |
| RAM          | ILM/DLM 512K, DDR 1.5G  |
| Acceleration | RVV 1.0, RVP 0.5.4    |

> - ILM/DLM need a 512K version bitstream.
> - Other Nuclei processor based chip can be easily supported.

# How to Use TinyMaix optimized for Nuclei RISC-V Processor

We provide different ways to explore the tinymaix examples, you can use it in following ways:

1. Use tinymaix with Nuclei SDK in terminal, using Nuclei SDK's make build system, and use example with Makefile,
   and enjoy the ease with Nuclei SDK.
2. Use tinymaix with Nuclei Studio IDE, just import the prebuilt tinymaix npk package, and use project wizard to
   create sample tinymaix example, and use it in IDE.

**Important Notice**: Most of the examples are not able to be built with default linker script with only 64K ILM/DLM
size, please change it to 512K ILM/DLM by hand, and make sure it match with your cpu ilm/dlm configuration in hardware.

## 1. Use TinyMaix in Terminal with Nuclei SDK

### Board

- [Nuclei DDR200T for 200/300 CPU Series](https://nucleisys.com/developboard.php#ddr200t)
- [Xilinx VCU118 for 900 Series](https://www.xilinx.com/products/boards-and-kits/vcu118.html)

> Support can be easily ported to other Nuclei RISC-V processor based CPUs.

### Development Environment

- [Nuclei SDK](https://github.com/Nuclei-Software/nuclei-sdk)

### Operation Steps

- Clone Nuclei SDK

~~~shell
git clone https://github.com/Nuclei-Software/nuclei-sdk
# export NUCLEI_SDK_ROOT environment variable is required
export NUCLEI_SDK_ROOT=$(readlink -f nuclei-sdk)
~~~

Make sure you have followed the steps in https://doc.nucleisys.com/nuclei_sdk/quickstart.html#use-prebuilt-tools-in-nuclei-studio setup toolchain environment.

- Clone Tinymaix ported for Nuclei

~~~shell
# branch: nuclei-main
git clone -b nuclei-main https://github.com/Nuclei-Software/npk-tinymaix.git
~~~

- Build and run tinymaix examples

> Currently supported cases are cifar10/kws/mnist/mbnet/vww

Take cifar10 as example using Nuclei DDR200T board, N300 RISC-V CPU.

> Since the examples are ported to nuclei-sdk, so you can easily
> use it just like sdk application, see guide here
> https://doc.nucleisys.com/nuclei_sdk/quickstart.html#build-run-and-debug-sample-application

Here in these examples' Makefile, if you built with `DOWNLOAD=ilm`, build system will use a 512K ilm/dlm linker script file,
see `examples/gcc_512K.ld` and `examples/Makefile.common`, so if you want to change the linker script file, please change to
this one.

**Run on qemu (software simulation):**

~~~shell
cd TinyMaix/examples/cifar10/
# choose n300fd(rv32imafdc) as example
# DOWNLOAD mode support ilm and ddr, here use ilm mode
# clean and build project
make SOC=evalsoc CORE=n300fd DOWNLOAD=ilm clean all
# test it using qemu
make SOC=evalsoc CORE=n300fd DOWNLOAD=ilm run_qemu
~~~

**Run on FPGA Board:**

~~~shell
cd TinyMaix/examples/cifar10/
# choose n300fd(rv32imafdc) as example
# DOWNLOAD mode support ilm and ddr
# clean and build project
make SOC=evalsoc CORE=n300fd DOWNLOAD=ilm clean all
# connect fpga board, and program bitstream using xilinx vivado tools
# connect using hbird debugger to fpga board
# download program to fpga board, and monitor on com port
make SOC=evalsoc CORE=n300fd DOWNLOAD=ilm upload
~~~

## 2. Use TinyMaix in Nuclei Studio IDE

### Development Environment

- Nuclei SDK 0.6.0
- Nuclei Studio 2024.06
- TinyMaix 1.2.0

### Operation Steps

- Download Nuclei Studio IDE from [Nuclei Studio](https://www.rvmcu.com/nucleistudio.html)

- Open the Nuclei Studio IDE

- Download TinyMaix zip package from [TinyMaix](https://github.com/Nuclei-Software/npk-tinymaix/releases/tag/1.2.0)
  or `mwp-nsdk_tinymaix` package from Nuclei Package Management in Nuclei Studio IDE

- Download a Nuclei SDK package, version **0.6.0** from the Nuclei Package Management in Nuclei Studio IDE

  > **Note:**
  > Another way is supported that import SDK zip package which can be obtained from [Nuclei SDK](https://github.com/Nuclei-Software/nuclei-sdk) 
  > through Nuclei Package Management in the IDE. And make sure only one version of Nuclei SDK can be installed.

  ![import_sdk](images/import_sdk.png)

- Import the zip package of **TinyMaix** in the same way after the steps above are ready
- Create a new Nuclei RISC-V C/C++ Project (refer to the [Nuclei IDE User Guide](https://download.nucleisys.com/upload/files/doc/nucleistudio/Nuclei_Studio_User_Guide.202406.pdf) if necessary)

  a. Choose the SoC, board and the SDK.

  ![creat_c_project](images/creat_c_project.png)

   b. Find the example you want and fill the configuration items

  ![select_example_and_config](images/select_example_and_config.png)

  > **Note:**
  >
  > - Users can filter by tinymaix (or ai or tinyml) to find the example  more quickly；
  > - More information about the extension types may refer to [arch-ext](https://doc.nucleisys.com/nuclei_sdk/develop/buildsystem.html#arch-ext).

- Build and run

   a. Click the "Build" button to build the project

  > **Note:**
  > - Size for ilm and ram should be set big enough at least 512K as nessessary, or the compilation will fail；
  > - You need to change the `nuclei_sdk/SoC/evalsoc/Board/nuclei_fpga_eval/Source/GCC/evalsoc.memory` link script manually, change the value of `ILM_MEMORY_SIZE` and `DLM_MEMORY_SIZE` from 0x10000 to 0x80000；
  > - And make sure your cpu ilm/dlm configuration also match with the changes.

  ![build](images/build.png)

   b. Select qemu debugging and click the "Run" button

  ![run](images/run.png)

**Note:** If you meet an issue like this: `section .text will not fit in region ilm`, this is caused generally by ilm/dlm size not big enough to store the code,
          please change the ilm/dlm size from 64K/64K to 512K/512K. If run on hardware, please make sure the hardware is configured with 512K ILM/DLM.

~~~linkscript
# IDE: nuclei_sdk/SoC/evalsoc/Board/nuclei_fpga_eval/Source/GCC/evalsoc.memory
/* ILM Memory Information */
ILM_MEMORY_PRESENT = 1;
ILM_MEMORY_BASE = 0x80000000;
ILM_MEMORY_SIZE = 0x80000;

/* DLM Memory Information */
DLM_MEMORY_PRESENT = 1;
DLM_MEMORY_BASE = 0x90000000;
DLM_MEMORY_SIZE = 0x80000;
~~~

If 512K ILM/DLM still does not meet for some cases(such as mbnet), change the download mode to ddr, make sure the hardware support ddr if run on hardware.

![change_download_mode](images/change_download_mode.png)

## Result

### N300 series

- bitstream: n300_best_config_ku060_16M_e3f086144_8bc45f3df_202407151024.bit
- Board: KU060
- CPU clock: 16MHz

#### Example cifar10		

| RUNCONFIG |	rv32imafdcp |	rv32imafdc |	rv32imac | rv32imafc |
| -- | -- | -- | -- | -- |
| freq/HZ   | 16000614    | 16001597   | 15999959 | 16001269  |
| param/KB  | 88.4        | 88.4       | 88.4     | 88.4      |
| OPS/MOPS  | 3.08        | 3.08       | 3.08     | 3.08      |
| buffer/KB | 11.0        | 11.0       | 11.0     | 11.0      |
| time/ms   | 303.448     | 795.588    | 1076.420 | 799.886   |
| cycle     | 4855354     | 12730678   | 17222675 | 12799191  |

#### Example kws

| RUNCONFIG |	rv32imafdcp |	rv32imafdc |	rv32imac | rv32imafc |
| -- | -- | -- | -- | -- |
| freq/HZ      | 15999959    | 15993405   | 15999959 | 16001269  |
| param/KB     | 8.3         | 8.3        | 8.3      | 8.3       |
| OPS/MOPS     | 0.24        | 0.24       | 0.24     | 0.24      |
| buffer/KB    | 5.0         | 5.0        | 5.0      | 5.0       |
| time/ms      | 120.123     | 150.293    | 416.598  | 154.687   |
| cycle        | 1921963     | 2403696    | 6665550  | 2475188   |

#### Example mnist	

| RUNCONFIG |	rv32imafdcp |	rv32imafdc |	rv32imac | rv32imafc |
| -- | -- | -- | -- | -- |
| freq/HZ   | 16000614    | 16001269   | 16000614 | 16000286  |
| param/KB  | 1.9         | 1.9        | 1.9      | 1.9       |
| OPS/MOPS  | 0.02        | 0.02       | 0.02     | 0.02      |
| buffer/KB | 1.4         | 1.4        | 1.4      | 1.4       |
| time/ms   | 7.614       | 11.004     | 41.456   | 13.731    |
| cycle     | 121828      | 176077     | 663321   | 219699    |

#### Example vww		

| RUNCONFIG |	rv32imafdcp |	rv32imafdc |	rv32imac | rv32imafc |
| -- | -- | -- | -- | -- |
| freq/HZ   | 15999959    | 16001269   | 15999959  | 16001925  |
| param/KB  | 224.6       | 224.6      | 224.6     | 224.6     |
| OPS/MOPS  | 7.49        | 7.49       | 7.49      | 7.49      |
| buffer/KB | 54.0        | 54.0       | 54.0      | 54.0      |
| time/ms   | 1471.639    | 2766.867   | 10485.742 | 2790.640  |
| cycle     | 23546163    | 44273383   | 167771442 | 44655611  |

### N900 series

- bitstream: u900_best_config_vcu118_50M_c6e66aff6b_9b05e5052_202407230929.bit
- Board: VCU118
- CPU clock: 50MHz

#### Example cifar10	

| RUNCONFIG | rv32imafdcv_ddr | rv32imafdcp_ddr | rv32imafdc_ddr | rv32imafdcv_ilm | rv32imafdcp_ilm | rv32imafdc_ilm |
|-----------|-----------------|-----------------|----------------|-----------------|-----------------|----------------|
| freq/HZ   | 50328698        | 50327060        | 50328698       | 50307727        | 50306416        | 50328698       |
| param/KB  | 88.4            | 88.4            | 88.4           | 88.4            | 88.4            | 88.4           |
| OPS/MOPS  | 3.08            | 3.08            | 3.08           | 3.08            | 3.08            | 3.08           |
| buffer/KB | 11.0            | 11.0            | 11.0           | 11.0            | 11.0            | 11.0           |
| time/ms   | 36.602          | 111.678         | 140.927        | 33.821          | 108.664         | 137.567        |
| cycle     | 1842131         | 5620425         | 7092672        | 1701457         | 5466496         | 6923567        |

#### Example kws

| RUNCONFIG | rv32imafdcv_ddr | rv32imafdcp_ddr | rv32imafdc_ddr | rv32imafdcv_ilm | rv32imafdcp_ilm | rv32imafdc_ilm |
|-----------|-----------------|-----------------|----------------|-----------------|-----------------|----------------|
| freq/HZ   | 50328698        | 50327060        | 50328698       | 50328698        | 50327060        | 50328698       |
| param/KB  | 8.3             | 8.3             | 8.3            | 8.3             | 8.3             | 8.3            |
| OPS/MOPS  | 0.24            | 0.24            | 0.24           | 0.24            | 0.24            | 0.24           |
| buffer/KB | 5.0             | 5.0             | 5.0            | 5.0             | 5.0             | 5.0            |
| time/ms   | 23.186          | 33.972          | 35.159         | 24.102          | 33.790          | 33.634         |
| cycle     | 1166921         | 1709710         | 1769506        | 1213022         | 1700551         | 1692755        |

#### Example mbnet	

| RUNCONFIG | rv32imafdcv_ddr | rv32imafdcp_ddr | rv32imafdc_ddr |
|-----------|-----------------|-----------------|----------------|
| freq/HZ   | 50328698        | 50306416        | 50307727       |
| param/KB  | 481.9           | 481.9           | 481.9          |
| OPS/MOPS  | 13.58           | 13.58           | 13.58          |
| buffer/KB | 96.0            | 96.0            | 96.0           |
| time/ms   | 383.394         | 602.711         | 876.970        |
| cycle     | 19295720        | 30320230        | 44118367       |

#### Example mnist	

| RUNCONFIG | rv32imafdcv_ddr | rv32imafdcp_ddr | rv32imafdc_ddr | rv32imafdcv_ilm | rv32imafdcp_ilm | rv32imafdc_ilm |
|-----------|-----------------|-----------------|----------------|-----------------|-----------------|----------------|
| freq/HZ   | 50327060        | 50328698        | 50327060       | 50327060        | 50328698        | 50327060       |
| param/KB  | 1.9             | 1.9             | 1.9            | 1.9             | 1.9             | 1.9            |
| OPS/MOPS  | 0.02            | 0.02            | 0.02           | 0.02            | 0.02            | 0.02           |
| buffer/KB | 1.4             | 1.4             | 1.4            | 1.4             | 1.4             | 1.4            |
| time/ms   | 1.632           | 2.324           | 2.510          | 1.570           | 2.284           | 2.459          |
| cycle     | 82133           | 116963          | 126320         | 79013           | 114950          | 123754         |

#### Example vww	

| RUNCONFIG | rv32imafdcv_ddr | rv32imafdcp_ddr | rv32imafdc_ddr | rv32imafdcv_ilm | rv32imafdcp_ilm | rv32imafdc_ilm |
|-----------|-----------------|-----------------|----------------|-----------------|-----------------|----------------|
| freq/HZ   | 50328698        | 50327060        | 50307727       | 50328698        | 50327060        | 50307727       |
| param/KB  | 224.6           | 224.6           | 224.6          | 224.6           | 224.6           | 224.6          |
| OPS/MOPS  | 7.49            | 7.49            | 7.49           | 7.49            | 7.49            | 7.49           |
| buffer/KB | 54.0            | 54.0            | 54.0           | 54.0            | 54.0            | 54.0           |
| time/ms   | 300.367         | 466.724         | 581.984        | 294.253         | 460.156         | 577.571        |
| cycle     | 15117080        | 23488846        | 29278292       | 14809370        | 23158298        | 29056284       |

### UX900 series

- bitstream: ux900_best_config_vcu118_50M_c6e66aff6b_9b05e5052_202407212346.bit
- Board: VCU118
- CPU clock: 50MHz

#### Example cifar10	

| RUNCONFIG | rv64imafdcv_ddr | rv64imafdcp_ddr | rv64imafdc_ddr | rv64imafdcv_ilm | rv64imafdcp_ilm | rv64imafdc_ilm |
|-----------|-----------------|-----------------|----------------|-----------------|-----------------|----------------|
| freq/HZ   | 50322472        | 50322472        | 50322472       | 50322472        | 50322472        | 50322472       |
| param/KB  | 88.4            | 88.4            | 88.4           | 88.4            | 88.4            | 88.4           |
| OPS/MOPS  | 3.08            | 3.08            | 3.08           | 3.08            | 3.08            | 3.08           |
| buffer/KB | 11.0            | 11.0            | 11.0           | 11.0            | 11.0            | 11.0           |
| time/ms   | 35.361          | 98.098          | 140.772        | 33.501          | 95.081          | 137.811        |
| cycle     | 1779452         | 4936533         | 7083995        | 1685853         | 4784710         | 6934990        |

#### Example kws

| RUNCONFIG | rv64imafdcv_ddr | rv64imafdcp_ddr | rv64imafdc_ddr | rv64imafdcv_ilm | rv64imafdcp_ilm | rv64imafdc_ilm |
|-----------|-----------------|-----------------|----------------|-----------------|-----------------|----------------|
| freq/HZ   | 50322472        | 50322472        | 50322472       | 50322472        | 50322472        | 50322472       |
| param/KB  | 8.3             | 8.3             | 8.3            | 8.3             | 8.3             | 8.3            |
| OPS/MOPS  | 0.24            | 0.24            | 0.24           | 0.24            | 0.24            | 0.24           |
| buffer/KB | 5.0             | 5.0             | 5.0            | 5.0             | 5.0             | 5.0            |
| time/ms   | 24.290          | 36.207          | 36.006         | 24.430          | 36.124          | 35.978         |
| cycle     | 1222332         | 1822025         | 1811910        | 1229377         | 1817848         | 1810501        |

#### Example mbnet	

| RUNCONFIG | rv64imafdcv_ddr | rv64imafdcp_ddr | rv64imafdc_ddr |
|-----------|-----------------|-----------------|----------------|
| freq/HZ   | 50322472        | 50322472        | 50322472       |
| param/KB  | 481.9           | 481.9           | 481.9          |
| OPS/MOPS  | 13.58           | 13.58           | 13.58          |
| buffer/KB | 96.0            | 96.0            | 96.0           |
| time/ms   | 428.969         | 579.688         | 910.920        |
| cycle     | 21586780        | 29171333        | 45839746       |

#### Example mnist	

| RUNCONFIG | rv64imafdcv_ddr | rv64imafdcp_ddr | rv64imafdc_ddr | rv64imafdcv_ilm | rv64imafdcp_ilm | rv64imafdc_ilm |
|-----------|-----------------|-----------------|----------------|-----------------|-----------------|----------------|
| freq/HZ   | 50322472        | 50322472        | 50322472       | 50322472        | 50322472        | 50322472       |
| param/KB  | 1.9             | 1.9             | 1.9            | 1.9             | 1.9             | 1.9            |
| OPS/MOPS  | 0.02            | 0.02            | 0.02           | 0.02            | 0.02            | 0.02           |
| buffer/KB | 1.4             | 1.4             | 1.4            | 1.4             | 1.4             | 1.4            |
| time/ms   | 1.620           | 2.700           | 2.596          | 1.547           | 2.654           | 2.542          |
| cycle     | 81522           | 135870          | 130637         | 77848           | 133555          | 127919         |

#### Example vww	

| RUNCONFIG | rv64imafdcv_ddr | rv64imafdcp_ddr | rv64imafdc_ddr | rv64imafdcv_ilm | rv64imafdcp_ilm | rv64imafdc_ilm |
|-----------|-----------------|-----------------|----------------|-----------------|-----------------|----------------|
| freq/HZ   | 50322472        | 50322472        | 50322472       | 50322472        | 50322472        | 50322472       |
| param/KB  | 224.6           | 224.6           | 224.6          | 224.6           | 224.6           | 224.6          |
| OPS/MOPS  | 7.49            | 7.49            | 7.49           | 7.49            | 7.49            | 7.49           |
| buffer/KB | 54.0            | 54.0            | 54.0           | 54.0            | 54.0            | 54.0           |
| time/ms   | 331.818         | 423.680         | 605.406        | 319.787         | 415.162         | 598.610        |
| cycle     | 16697902        | 21320624        | 30465526       | 16092472        | 20891978        | 30123534       |

> **Note:** Other CPU series can be easily tested using Nuclei SDK
> using different fpga bitstream.

## FAQ

If you have questions or troubles when following this guideline, please check this issues,
if not solved, you can open a new issue, maybe community could help with you.

## Author

[Huaqi Fang](https://github.com/fanghuaqi)

[Jiuling Sun](https://github.com/JoleenSun)
