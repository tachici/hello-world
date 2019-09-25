
import java.util.ArrayList;
import java.util.concurrent.Callable;
import java.util.concurrent.Future;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * An update request acts as a writer thus it must:
 * 1. Request service.
 * 2. Needs exclusive access to the corresponding table.
 * 3. Release the service request.
 * 4. Insert
 * 5. Release access to the table.
 * @author Tachesi
 */
public class UpdateReq implements Callable<Object>{
    
    Database dataBase;
    String tableName;
    ArrayList<Object> values;
    String condition;
    //splitWork = 0 -> this updateReq is resposible for the entire table
    //splitWork = 1 -> start and end should be taken into consideration
    int splitWork;
    int start;
    int end;
    
    public UpdateReq(Database dataBase, String tableName, ArrayList<Object> values, String condition) {
        this.dataBase = dataBase;
        this.tableName = tableName;
        this.values = values;
        this.condition = condition;
        this.splitWork = 0;
    }
    
    public UpdateReq(Database dataBase, String tableName, ArrayList<Object> values, String condition, int start, int end) {
        this.dataBase = dataBase;
        this.tableName = tableName;
        this.values = values;
        this.condition = condition;
        this.splitWork = 0;
        this.start = start;
        this.end = end;
        this.splitWork = 1;
    }
    
    //update(tableName, values, condition)
    @Override
    public Object call() throws Exception {
        
        
        
        //insert new values by itself if there is no need for spliting:
        Table table = dataBase.tables.get(tableName);
        if (splitWork == 1) {
            //a worker thread needs to take care of the calculations:
            
            //do all the updates
            for (int i = start; i < end; i++) {
                if (ConditionChecker.checkCondition(table, table.get(i), condition)) {
                    
                    table.set(i, values);
                }
            }
            
            //release resources:
            //dataBase.tables.get(tableName).resourceAccess.release();
            
        } else {
            //check if the work should be or not splitted among workers:
            if (table.size() < 10 || dataBase.nrThreads == 1) {
                //no split is required:
                UpdateReq req = new UpdateReq(dataBase, tableName, values, condition, 0, table.size());
                Future<Object> res = dataBase.exeService.submit(req);
                res.get();
            } else {
                //the work should be splitted among nrThreads:
                int entriesPerThread = table.size() / dataBase.nrThreads;
                UpdateReq[] reqs = new UpdateReq[dataBase.nrThreads];
                Future[] res = new Future[dataBase.nrThreads];
                for (int i = 0; i < dataBase.nrThreads - 1; i++) {
                    reqs[i] = new UpdateReq(dataBase, tableName, values, condition, i * entriesPerThread, (i + 1) * entriesPerThread);
                    res[i] = dataBase.exeService.submit(reqs[i]);
                }
                //the last one will do all the remaining entries:
                reqs[dataBase.nrThreads - 1] = new UpdateReq(dataBase, tableName, values, condition, (dataBase.nrThreads - 1) * entriesPerThread, table.size());
                res[dataBase.nrThreads - 1] = dataBase.exeService.submit(reqs[dataBase.nrThreads - 1]);
                
                for (int i = 0; i < dataBase.nrThreads; i++) {
                    
                    res[i].get();
                }
            }
        }
        
        
        
        return null;
    }
    
}