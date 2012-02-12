EXECUTE_PROCESS( COMMAND groupadd kueued
                 COMMAND useradd -c "kueued user" -s /bin/false -G kueued kueued )
		 
IF( NOT EXISTS "/var/log/kueued-srv.log" )

    EXECUTE_PROCESS( COMMAND touch /var/log/kueued-srv.log
                     COMMAND chown kueued /var/log/kueued-srv.log )
		 
ENDIF( NOT EXISTS "/var/log/kueued-srv.log" )
                 
IF( NOT EXISTS "/var/log/kueued-db.log" )

    EXECUTE_PROCESS( COMMAND touch /var/log/kueued-db.log
		     COMMAND chown kueued /var/log/kueued-db.log )

ENDIF( NOT EXISTS "/var/log/kueued-db.log" ) 


