<?php
/**
 * Implementation of hook_b_command().
 */
function help_b_command() {
  $items['help'] = array(
    'description' => 'Display help.',
    'arguments' => array(
      'command' => 'Command to print help',
    ),
    'callback' => 'help_b_callback',
  );
  return $items;
}

function help_b_callback($arguments, $options) {
  $commands = b_get_commands();
//  print_r($commands);
  $rows = array();
  $answer = array();
  if(isset($arguments['command'])){
    $cmd = $arguments['command'];
    if(!isset($commands[$cmd])){
      return array(
        '#type' => 'text',
        '#color' => 'red',
        'value' => 'There is no ' . $cmd,
      );
    }
    $output = display_help_by_cmd($commands[$cmd]);
  }
  else{
    foreach($commands as $cmd => $callbacks){
      if($elements = display_help_command($callbacks)) {
        $rows[] = array(
          array( 'data' => $cmd),
          $elements
        );
      }
    }
    $output[] = array(
      '#type' => 'text',
      '#color' => 'purple',
      'value' => "\nB commands:\n",
    );
    
    $output[] = array(
      '#type' => 'table',
      'rows' => $rows,
      'delimiter' => '    ',
      'delimiter_left' => ' ',
      'delimiter_right' => '',    
    );
  }
  
  return $output;
}

function display_help_by_cmd($commands){
  $examples = array();
  $available_options = array();
  $available_arguments = array();
  foreach($commands as $callback){
    if(isset($callback['examples'])){
      foreach($callback['examples'] as $example => $description){
        $examples[] = array(
          $example,
          $description
        );
      }
    }
    if(isset($callback['options'])){
      foreach($callback['options'] as $option => $description){
        $available_options[] = array(
          '--' . $option,
          $description
        );
      }
    }
    if(isset($callback['arguments'])){
      foreach($callback['arguments'] as $argument => $description){
        $available_arguments[] = array(
          $argument,
          $description
        );
      }
    }
  }
  $output = array();
    $output[] = array(
      '#type' => 'text',
      '#color' => 'bold_gray',
      'value' => $callback['description'],
    );

  if(!empty($examples)){
    $output[] = array(
      '#type' => 'text',
      '#color' => 'blue',
      'value' => "\nExamples:",
    );
    $output[] = array(
      '#type' => 'table',
      'rows' => $examples,
      'delimiter' => ' : ',
      'delimiter_left' => ' ',
      'delimiter_right' => '',    
    );
  }
  if(!empty($available_options)){
    $output[] = array(
      '#type' => 'text',
      '#color' => 'blue',
      'value' => "\nOptions:",
    );
    $output[] = array(
      '#type' => 'table',
      'rows' => $available_options,
      'delimiter' => ' : ',
      'delimiter_left' => ' ',
      'delimiter_right' => '',    
    );
  }
  if(!empty($available_arguments)){
    $output[] = array(
      '#type' => 'text',
      '#color' => 'blue',
      'value' => "\nArguments:",
    );
    $output[] = array(
      '#type' => 'table',
      'rows' => $available_arguments,
      'delimiter' => ' : ',
      'delimiter_left' => ' ',
      'delimiter_right' => '',    
    );
  }
  return $output;
}

function display_help_command($callbacks) {
  $rows = array();
  foreach($callbacks as $callback){
    if(isset($callback['is_alias'])){
      continue;
    }
    $rows[] = $callback['description'];
  }
  if(empty($rows)){
    return FALSE;
  }
  return implode("\n", $rows);
}