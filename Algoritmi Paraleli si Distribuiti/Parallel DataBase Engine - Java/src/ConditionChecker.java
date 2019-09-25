
import java.util.ArrayList;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author Tachesi
 */
public class ConditionChecker {
    
    /**
     * Check if an entry satisfies a condition.
     * A condition has the form:
     * columnName comparator value
     * Comparator can be: <, >, ==
     * The type of value is the type of column.
     * @param table
     * @param tableEntry
     * @param condition types accepted: “string”, “int” or “bool”
     * @return true if entry satisfies a condition.
     */
    static boolean checkCondition(Table table, ArrayList<Object> tableEntry, String condition) {
        
        //firstly check if the condition is not empty:
        if (condition.equals("")) {
            return true;
        }
        
        String[] condWords = condition.split("\\s+");
        String columnName = condWords[0];
        String comparator = condWords[1];
        String value = condWords[2];
        
        int column = table.columnIndexes.get(columnName);
        
        
        switch(comparator) {
            case "==":
                //do different stuff for different types:
                switch(table.columnTypes[column]) {
                    case "bool":
                        boolean valueB = Boolean.getBoolean(value);
                        return (valueB == (Boolean)tableEntry.get(column));
                    case "int":
                        int valueI = Integer.valueOf(value);
                        return (valueI == (Integer)tableEntry.get(column));
                    case "string":
                        
                        return value.equals(tableEntry.get(column).toString());
                }
                break;
            
            case ">":
                switch(table.columnTypes[column]) {
                    case "int":
                        int valueI = Integer.valueOf(value);
                        return (Integer)tableEntry.get(column) > valueI;
                    default:
                        throw new RuntimeException("> does not support type String");
                }
            
            case "<":
            switch(table.columnTypes[column]) {
                case "int":
                    int valueI = Integer.valueOf(value);
                    return (Integer)tableEntry.get(column) < valueI;
                default:
                    throw new RuntimeException("< does not support type String");
            }
            
            //no condition:
            case "":
                return true;
            default:
                throw new RuntimeException("Operation: " + comparator + " Not supported!");
        }
        
        throw new RuntimeException("Operation: " + comparator + " Not supported!");
    }
}
