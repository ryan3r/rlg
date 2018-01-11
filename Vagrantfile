# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|

  config.vm.box = "ubuntu/trusty64"

  config.vm.synced_folder ".", "/workspace"

  config.vm.provision "shell", inline: <<-SHELL
    sudo apt-get update -y
    sudo apt-get install -y libc6-dbg vim g++ gdb ncurses-devel git

    cd /tmp
    echo "Downloading valgrind"
    wget ftp://sourceware.org/pub/valgrind/valgrind-3.13.0.tar.bz2 -q
    tar xf valgrind-3.13.0.tar.bz2
    cd valgrind-3.13.0
    ./configure
    make
    sudo make install
    rm -r /tmp/valgrind-3.13.0.tar.bz2 /tmp/valgrind-3.13.0

    echo "" >> /home/vagrant/.bashrc
    echo "cd /workspace" >> /home/vagrant/.bashrc
    echo "PS1='${debian_chroot:+($debian_chroot)}\\[\\033[01;36m\\]\\w\\[\\033[00m\\] \\$ '" >> /home/vagrant/.bashrc

    git config --global user.email "ryanr@iastate.edu"
    git config --global user.name "Ryan Ray"
  SHELL
end
