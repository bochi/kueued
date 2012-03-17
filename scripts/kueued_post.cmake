EXECUTE_PROCESS( COMMAND groupadd kueued
                 COMMAND useradd -c "kueued user" -s /bin/false -G kueued kueued )
		 
IF( NOT EXISTS "/var/log/kueued/" )

	MAKE_DIRECTORY( /var/log/kueued )
	EXECUTE_PROCESS( COMMAND chown -R kueued /var/log/kueued )

ENDIF( NOT EXISTS "/var/log/kueued/" )

IF( NOT EXISTS "/var/log/kueued/kueued.log" )

    EXECUTE_PROCESS( COMMAND touch /var/log/kueued/kueued.log )
		 
ENDIF( NOT EXISTS "/var/log/kueued/kueued.log" )
                 
IF( NOT EXISTS "/var/log/kueued/kueued-debug.log" )

    EXECUTE_PROCESS( COMMAND touch /var/log/kueued/kueued-debug.log )

ENDIF( NOT EXISTS "/var/log/kueued/kueued-debug.log" ) 

IF( NOT EXISTS "/var/log/kueued/kueued-queries.log" )

    EXECUTE_PROCESS( COMMAND touch /var/log/kueued/kueued-queries.log )

ENDIF( NOT EXISTS "/var/log/kueued/kueued-queries.log" )  

EXECUTE_PROCESS( COMMAND chown kueued /var/log/kueued/kueued.log 
	         COMMAND chown kueued /var/log/kueued/kueued-queries.log 
		 COMMAND chown kueued /var/log/kueued/kueued-debug.log )


