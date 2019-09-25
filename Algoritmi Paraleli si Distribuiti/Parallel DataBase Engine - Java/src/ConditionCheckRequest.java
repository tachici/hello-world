
import java.util.ArrayList;
import java.util.concurrent.Callable;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 * Check if a part of the table given respects
 * the condition.
 * Return the indices that respect it.
 * @author Tachesi
 */
public class ConditionCheckRequest implements Callable<ArrayList<Integer>>{
    int start;
    int end;   
    Table table;
    String condition;
    ArrayList<Integer> goodEntries;
    
    public ConditionCheckRequest(Table table, String condition,int start, int end) {
        this.start = start;
        this.end = end;
        this.table = table;
        this.condition = condition;
        this.goodEntries = new ArrayList<>();
    }
    
    
    @Override
    public ArrayList<Integer> call() throws Exception {
        for (int i = start; i < end; i++) {
            if (ConditionChecker.checkCondition(table, table.get(i), condition)) {
                goodEntries.add(i);
            }
        }
        return goodEntries;
    }
    
}
