# HomeAutomation

Installation on Raspberrypi
---------------------------

1. git clone https://github.com/etimou/HomeAutomation
2. sudo apt-get install python-serial lighttpd
3. sudo cp /etc/lighttpd/lighttpd.conf /etc/lighttpd/lighttpd.conf.sav
4. sudo nano /etc/lighttpd/lighttpd.conf
   replace "/var/www/html" by "/home/pi/HomeAutomation/software/raspberrypi/www"
   add "mod_cgi" in the server.modules section

   add
   cgi.assign                 = ( ".pl"  => "/usr/bin/perl",
                               ".cgi" => "/usr/bin/perl",
                               ".rb"  => "/usr/bin/ruby",
                               ".erb" => "/usr/bin/eruby",
                               ".py"  => "/usr/bin/python",
                               ".php" => "/usr/bin/php-cgi" )

5. chmod o+w /home/pi/HomeAutomation/software/raspberrypi/MyHouse.db
6. chmod o+x /home/pi/HomeAutomation/software/raspberrypi/www/cgi-bin/*.py
7. sudo usermod -aG dialout www-data
8. sudo service lighttpd restart



