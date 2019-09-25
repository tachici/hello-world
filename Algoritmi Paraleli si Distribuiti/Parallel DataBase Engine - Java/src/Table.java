

import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.ReentrantLock;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */



/**
 * A table contains a list of entries.
 * An entry in a table is a list of Objects.
 * Only the table knows their interpretations.
 * No two threads are allowed to access the table,
 * if the table is in transactionMode.
 * @author Tachesi
 */
public class Table extends ArrayList<ArrayList<Object>>{
    String tableName;
    String[] columnNames;
    String[] columnTypes;
    HashMap<String, Integer> columnIndexes;
    ReentrantLock tableAccess;
    
    int readersNumber;
    Semaphore resourceAccess;   
    Semaphore readCountAccess;  //controls acces to nr readers     
    Semaphore serviceQueue;         
    //on transcationStarted = 1 all threads will require
    //to be able to aquire tableAccess
    int transactionStarted = 0;
    // 0 - no transaction
    // 1 - transaction going on
    
    public Table(String tableName, String[] columnNames, String[] columnTypes) {
        super();

        this.tableName = tableName;
        this.columnNames = columnNames;
        this.columnTypes = columnTypes;
        columnIndexes = new HashMap<>();
        
        //hashIndexes:
        for (int i = 0; i < columnNames.length; i++) {
            columnIndexes.put(columnNames[i], i);
        }
        
        //initialise all semaphores
        readersNumber = 0;
        resourceAccess = new Semaphore(1);
        readCountAccess = new Semaphore(1);
        serviceQueue = new Semaphore(1);
        tableAccess = new ReentrantLock(true);
    }
    
    
    void setTransaction(boolean x) throws InterruptedException {
        if (x == true) {
            transactionStarted = 1;
            tableAccess.lock();            
        } else {
            transactionStarted = 0;
            tableAccess.unlock();
        }
    }
    
    
}
