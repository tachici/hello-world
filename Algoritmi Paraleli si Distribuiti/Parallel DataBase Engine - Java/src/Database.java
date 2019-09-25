
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * Database class.
 * Holds an ExecutorService witch maintains a number of
 * threads given in the initDb function.
 * Holds tables in a HashMap where:
 *      Key - is a String
 *      Value - is a table.
 * 
 * @author Tachesi
 */
public class Database implements MyDatabase{
    ExecutorService exeService;
    HashMap<String, Table> tables;
    static boolean serialDatabase = false;  //debug purpose
    int nrThreads = 0;
    
    public Database() {
        tables = new LinkedHashMap<>();
    }

    @Override
    public void initDb(int numWorkerThreads) {
        //TEST:
        exeService = new ThreadPoolExecutor(numWorkerThreads, numWorkerThreads, 10, TimeUnit.HOURS, new ArrayBlockingQueue<>(1000));
        nrThreads = numWorkerThreads;
    }

    @Override
    public void stopDb() {
        exeService.shutdown();
    }

    @Override
    public void createTable(String tableName, String[] columnNames, String[] columnTypes) {
        tables.put(tableName, new Table(tableName, columnNames, columnTypes));
    }

    @Override
    public ArrayList<ArrayList<Object>> select(String tableName, String[] operations, String condition) {
        getPermision(tableName);
        
        
        ArrayList<ArrayList<Object>> response = null;
        SelectReq req = new SelectReq(exeService, this, tableName, operations, condition);

        try {
            response = req.call();
        } catch (Exception e) {
            System.out.println("Exception in select: + " + e.toString());
        }

        return response;

    }

    @Override
    public void update(String tableName, ArrayList<Object> values, String condition) {
        getPermision(tableName);
        startTransaction(tableName);
        UpdateReq req = new UpdateReq(this, tableName, values, condition);
        try {
            req.call();
        } catch(Exception e) {
            System.out.println("Error in update req: " + e.toString());
            e.printStackTrace();
            
        }
        endTransaction(tableName);
    }

    @Override
    public void insert(String tableName, ArrayList<Object> values) {
        getPermision(tableName);
        InsertReq req = new InsertReq(this, tableName, values);
        try {
            req.call();
        } catch (Exception ex) {
            Logger.getLogger(Database.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    @Override
    public void startTransaction(String tableName) {
        getPermision(tableName);
        Table table = tables.get(tableName);
        try {
            table.setTransaction(true);
        } catch (InterruptedException ex) {
            Logger.getLogger(Database.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    @Override
    public void endTransaction(String tableName) {
        Table table = tables.get(tableName);
        try {
            table.setTransaction(false);
        } catch (InterruptedException ex) {
            Logger.getLogger(Database.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    private void getPermision(String tableName) {
        Table table = tables.get(tableName);
        //wait for transaction to end:
        table.tableAccess.lock();
        table.tableAccess.unlock();
    }
    
    @Override
    public String toString() {
        String s = "";
        ArrayList<Table> db = new ArrayList<>(tables.values());
        for (int i = 0; i < db.size(); i++) {
            s = s + " Table: " + db.get(i).tableName + "\n";
            for (int j = 0; j < db.get(i).size(); j++) {
                s = s + " " + db.get(i).get(j) + "\n";
            }
            s = s + "\n";
        }
        return s;
    }
    
    
    
}
