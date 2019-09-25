
import java.util.ArrayList;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * A select request acts as a READER.
 * Needs the database and it's ExecutorService.
 * It will submit additional requests to the ExecutorService,
 * depending on the number of operations.
 * Each operation given will be transformed in
 * a request sent to the ExecutorService given.
 * @author Tachesi
 */
public class SelectReq implements Callable<ArrayList<ArrayList<Object>>>{
    Database dataBase;
    ExecutorService exeService;
    
    //select parameters
    String tableName;
    String[] operations;
    String condition;
    
    //return value:
    ArrayList<ArrayList<Object>> response;
    
    public SelectReq(ExecutorService exeService, Database dataBase, String tableName, String[] operations, String condition) {
        this.dataBase = dataBase;
        this.tableName = tableName;
        this.operations = operations;
        this.condition = condition;
        this.exeService = exeService;
        response = new ArrayList<>();
    }
    
    int sumArray(ArrayList<Integer> ints) {
        int sum = 0;
        for (Integer x : ints) {
            sum += x;
        }
        return sum;
    }
    
    private ArrayList<Object> assembleResponse(ArrayList<Future<ArrayList<Object>>> threadsResponses, String operation) throws InterruptedException, ExecutionException {
        ArrayList<Object> res = new ArrayList<>();
        
        //check if operation is a column:
        if (operation.contains("(") == false) {
            for (int i = 0; i < threadsResponses.size(); i++) {
                ArrayList<Object> computedResult = threadsResponses.get(i).get();
                res.addAll(computedResult);
            }
            return res;
        }
        
        operation = operation.substring(0, operation.indexOf("("));
        
        ArrayList<Integer> computedResult = new ArrayList<>();
        //operation is a: sum|avg|count|min|max
        for (Future<ArrayList<Object>> x : threadsResponses) {
            computedResult.add((Integer)x.get().get(0));
        }
        
        switch (operation) {
            case "sum":
                Integer sum = sumArray(computedResult);
                res.add(sum);
                break;
            case "count":
                Integer count = sumArray(computedResult);
                res.add(count);
                break;
            case "avg":
                Integer sumAvg = sumArray(computedResult);
                res.add(sumAvg / computedResult.size());
                break;
            case "min":
                int min = Integer.MAX_VALUE;
                for (Integer x : computedResult) {
                    if (x < min) {
                        min = x;
                    }
                }
                res.add(min);
                break;
            case "max":
                int max = 0;
                for (Integer x : computedResult) {
                    if (max < x) {
                        max = x;
                    }
                }
                res.add(max);
                break;
        }
        
        return res;
    }
    
    /**
     * @return An arrayList which contains other ArrayList based on an
     * operation given.
     * @throws Exception 
     */
    @Override
    public ArrayList<ArrayList<Object>> call() throws Exception {
        Table table = dataBase.tables.get(tableName);
        
        //table.serviceQueue.acquire();
        table.readCountAccess.acquire(); // request exclusive access to number of readers
        if (table.readersNumber == 0) {
            table.resourceAccess.acquire();
        }
        table.readersNumber++; // update count of active readers
        //table.serviceQueue.release();
        table.readCountAccess.release();
        
        //indexes of the entries that respect the condition:
        ArrayList<Integer> indexes = new ArrayList<>();
        ArrayList<Future<ArrayList<Integer>>> futureIndexes = new ArrayList<>();
        //split the condition checking to multiple threads:
        {
            int entriesPerThread = table.size() / dataBase.nrThreads;
            int start = 0;
            int end = entriesPerThread;
            if (table.size() > dataBase.nrThreads * 2)
                    for (int j = 0; j < dataBase.nrThreads - 1; j++) {
                        ConditionCheckRequest req = new ConditionCheckRequest(table, condition, start, end);
                        futureIndexes.add(dataBase.exeService.submit(req));
                        start += entriesPerThread;
                        end += entriesPerThread;
                    }
           //the final part of the entries:
           ConditionCheckRequest req = new ConditionCheckRequest(table, condition, start, table.size());
           futureIndexes.add(dataBase.exeService.submit(req));    
           
           //add the resulting indexes together:
           for (Future<ArrayList<Integer>> x : futureIndexes) {
               indexes.addAll(x.get());
           }
        }
        
        //DO THE READING:
        //for every requested OPERATION submit nrThreads * OperationExecutor:
        ArrayList<ArrayList<Future<ArrayList<Object>>>> futureRes = new ArrayList<>();
        for (int i = 0; i < operations.length; i++) {          
            futureRes.add(i, new ArrayList<>());
            
            //split every operation:
            //for every thread:
            int entriesPerThread = indexes.size() / dataBase.nrThreads;
            int start = 0;
            int end = entriesPerThread;
            
            
            if (indexes.size() > dataBase.nrThreads * 2)
                for (int j = 0; j < dataBase.nrThreads - 1; j++) {
                    OperationExecutor exe = new OperationExecutor(exeService, dataBase, tableName, operations[i], condition, indexes, start, end);
                    futureRes.get(i).add(dataBase.exeService.submit(exe));
                    start += entriesPerThread;
                    end += entriesPerThread;
                }
            //the final part of the entries:
            OperationExecutor exe = new OperationExecutor(exeService, dataBase, tableName, operations[i], condition, indexes, start, indexes.size());
            futureRes.get(i).add(dataBase.exeService.submit(exe));     
        }
        

        for (int i = 0; i < operations.length; i++) {
             //asemble response:
             response.add(assembleResponse(futureRes.get(i), operations[i]));
        }

       table.readCountAccess.acquire();        // request exclusive access to readCount
        // <EXIT>
        table.readersNumber--;
        if (table.readersNumber == 0) {
            table.resourceAccess.release();   // release resource access for all
        }
        table.readCountAccess.release();
        
        return response;
    }
    
}
