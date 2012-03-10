EXECUTE_PROCESS( COMMAND groupadd kueued
                 COMMAND useradd -c "kueued user" -s /bin/false -G kueued kueued )
		 
IF( NOT EXISTS "/var/log/kueued/" )

	MAKE_DIRECTORY( /var/log/kueued )
	EXECUTE_PROCESS( COMMAND chown -R kueued /var/log/kueued )

ENDIF( NOT EXISTS "/var/log/kueued/" )

IF( NOT EXISTS "/var/log/kueued/kueued.log" )

    EXECUTE_PROCESS( COMMAND touch /var/log/kueued/kueued.log
                     COMMAND chown kueued /var/log/kueued/kueued.log )
		 
ENDIF( NOT EXISTS "/var/log/kueued/kueued.log" )
                 
IF( NOT EXISTS "/var/log/kueued/kueued-dbupdate.log" )

    EXECUTE_PROCESS( COMMAND touch /var/log/kueued/kueued-dbupdate.log
		     COMMAND chown kueued /var/log/kueued/kueued-dbupdate.log )

ENDIF( NOT EXISTS "/var/log/kueued/kueued-dbupdate.log" ) 


