
import java.util.ArrayList;
import java.util.concurrent.Callable;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * An insert request acts as a WRITER thus it must:
 * 1. Request service.
 * 2. Needs exclusive access to the corresponding table.
 * 3. Release the service request.
 * 4. Insert
 * 5. Release access to the table.
 * @author Tachesi
 */
public class InsertReq implements Callable<Object>{
    
    Database dataBase;
    String tableName;
    ArrayList<Object> values;

    public InsertReq(Database dataBase, String tableName, ArrayList<Object> values) {
        this.dataBase = dataBase;
        this.tableName = tableName;
        this.values = values;
    }
    
    @Override
    public Object call() throws Exception {
        
        //Request service.
        //dataBase.tables.get(tableName).serviceQueue.acquire();
        
        //Needs exclusive access to the corresponding table.
        dataBase.tables.get(tableName).resourceAccess.acquire();
         
        //Release the service request.
        //dataBase.tables.get(tableName).serviceQueue.release();

        //insert new values
        dataBase.tables.get(tableName).add(values);
        
        //release resources:
        dataBase.tables.get(tableName).resourceAccess.release();
        
        return null;
    }
    
}
