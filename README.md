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

5. sudo usermod -aG dialout www-data
6. sudo service lighttpd restart



