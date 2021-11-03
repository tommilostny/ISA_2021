service xinetd start  #也可使用`restart`;
service tftpd-hpa start;
netstat -a | grep tftp;