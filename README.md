# 💾 Disk Studio 

**Disk Studio ** is an advanced disk management tool developed for Linux systems using C++ and Qt (Widgets), combining the best features of popular tools like GParted, Clonezilla, and Balena Etcher under a single roof.


<img width="1857" height="475" alt="image" src="https://github.com/user-attachments/assets/5eaec144-cec8-44db-81bc-7cecf89c5d82" />

## 🚀 Features

Disk Studio  offers powerful tools to simplify disk management:

* **📊 Visual Disk Map:** View disk partitions and usage rates on a colorful and interactive strip similar to GParted.
* **📋 Detailed Partition List:** View file systems (ext4, ntfs, fat32, exfat), mount points, UUIDs, and usage rates (with visual bars) in a detailed table.
* **💿 Disk Cloning:** Copy a disk sector-by-sector to another disk (based on `dd`).
    * *Safety:* Prevents the operation if the target disk is smaller than the source disk.
    * *UUID Randomizer:* Automatically changes the UUIDs of Linux partitions on the new disk to prevent conflicts after cloning.
* **🔥 ISO Writer (Etcher Mode):** Burn Linux or Windows ISO files to USB drives in a bootable format.
* **♻️ USB Restore:** Return USB drives that appear corrupted or show incorrect capacity after writing an ISO back to factory settings (Single partition FAT32/NTFS/exFAT) with a single click.
* **🛠️ Partitioning Operations:**
    * **Formatting:** Support for EXT4, NTFS, FAT32, exFAT.
    * **Resizing:** Visually shrink or grow partitions using a slider.
    * **Deleting:** Securely delete partitions.
    * **Renaming:** Change disk labels.
    * **Mount/Unmount:** Mount or unmount disks with a single click.

## 📦 Requirements

To compile and run the project, the following packages must be installed on your system (for Ubuntu/Debian based systems):

### Build Tools
```bash
sudo apt update
sudo apt install build-essential qt5-base-dev qt5-base-dev-tools
sudo apt install parted util-linux udisks2 ntfs-3g exfatprogs dosfstools e2fsprogs
