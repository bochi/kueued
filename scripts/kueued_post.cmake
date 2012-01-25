EXECUTE_PROCESS( COMMAND groupadd kueued
	         COMMAND useradd -c "kueued user" -d /etc/kueued -s /bin/false -G kueued kueued
		 COMMAND chown -R kueued /etc/kueued
		 COMMAND chgrp -R kueued /etc/kueued
		 COMMAND chown kueued /var/log/kueued.log 
		 COMMAND chgrp kueued /var/log/kueued.log )

	      
