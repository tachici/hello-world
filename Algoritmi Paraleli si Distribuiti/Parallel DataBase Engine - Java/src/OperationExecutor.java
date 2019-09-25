
import java.util.ArrayList;
import java.util.Collections;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * It will return a response containing the calculations
 * done on that part of the vector.
 * @author Tachesi
 */
public class OperationExecutor implements Callable<ArrayList<Object>>{
    
    ArrayList<Object> response;
    ArrayList<Integer> indexes;
    ExecutorService exeService;
    Database dataBase;
    String tableName;
    String operation;
    String condition;
    int start;
    int end;
    
    public OperationExecutor(ExecutorService exeService, Database dataBase, String tableName, String operation, String condition, 
            ArrayList<Integer> indexes ,int start, int end) {
        
        this.exeService = exeService;
        this.dataBase = dataBase;
        this.tableName = tableName;
        this.operation = operation;
        this.condition = condition;
        this.start = start;
        this.end = end;
        this.response = new ArrayList<>();
        this.indexes = indexes;
    }

    @Override
    public ArrayList<Object> call() throws Exception {
        
        //System.out.println("Operationexecutor: " + this + " Called.");
        
        Table table = dataBase.tables.get(tableName);
        
        //check firstly if it is a column operation only.
        if (operation.contains("(") == false) {
            String columnName = operation;
            int columnIndex = table.columnIndexes.get(columnName);
            for (int i = start; i < end; i++) {
               if(ConditionChecker.checkCondition(table, (ArrayList<Object>)table.get(indexes.get(i)), condition)) {
                   response.add(table.get(indexes.get(i)).get(columnIndex));
               }
            }
            
            //return the table entries which respected the condition:
            return response;
        }
        
        
        String columnName = operation.substring(operation.indexOf("(") + 1, operation.indexOf(")"));
        String operationType = operation.substring(0, operation.indexOf("("));
        int columnIndex = table.columnIndexes.get(columnName);
        
        
        
        //count will work on any type of entry:
        if (operationType.equals("count")) {
            response.add(end - start);
            return response;
        }
        
        //if it is any other operation get the ints:
        ArrayList<Integer> listOfInts = new ArrayList<>();
        for (int i = start; i < end; i++) {
            listOfInts.add((Integer)((ArrayList<Object>)table.get(indexes.get(i))).get(columnIndex));
        }
        
        //in case there aren't any valid entries
        if (listOfInts.size() == 0) {
            response.add(0);
            return response;
        }
        
        switch (operationType) {
            case "max":
                response.add(Collections.max(listOfInts));
                break;
            case "min":
                response.add(Collections.min(listOfInts));
                break;
            case "avg":
                int sum = 0;
                for (int i = 0; i < listOfInts.size(); i++) {
                    sum += listOfInts.get(i);
                }
                response.add(sum / listOfInts.size());
                break;
            case "sum":
                sum = 0;
                sum = listOfInts.stream().map((x) -> x).reduce(sum, Integer::sum);
                
                response.add(sum);
                break;
            case "count":
                response.add(listOfInts.size());
                break;
            default:
                throw new RuntimeException("This operation type is not supported! " + operationType);
        }     
        return response;
    }
    
    
}
