<?php
/**
 * Implementation of hook_b_command().
 */
function modules_b_command() {
  $items['pm-list'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Show a list of available extensions (modules, layouts or themes).',
    'options' => array(
      'root' => 'Backdrop ROOT directory',
      'type' => 'Filter by extension type. Choices: module, theme, layout.',
    ),
    'callback' => 'modules_b_list_callback',
    'aliases' => array('pml'),
  );
  $items['pm-enable'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Enable one or more extensions (modules, layouts or themes).',
    'arguments' => array(
      'extensions' => 'A list of modules or themes. You can use the * wildcard at the end of extension names to disable multiple matches.',
    ),
    'options' => array(
      'root' => 'Backdrop ROOT directory',
    ),
    'callback' => 'modules_b_enable_callback',
    'aliases' => array('en'),
  ); 
  $items['pm-disable'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Disable one or more extensions (modules, layouts or themes).',
    'arguments' => array(
      'extensions' => 'A list of modules or themes. You can use the * wildcard at the end of extension names to disable multiple matches.',
    ),
    'options' => array(
      'root' => 'Backdrop ROOT directory',
    ),    
    'aliases' => array('dis'),
    'callback' => 'modules_b_disable_callback',
  ); 
  return $items;
}

function modules_b_list_callback($arguments, $options){

  if(!isset($options['type']) or $options['type'] != 'layout') {
    $query = db_select('system', 's')
      ->fields('s');
    
    if(isset($options['type']) && in_array($options['type'], array('module', 'theme'))) {
      $query->condition('type', $options['type']);
    }
      
    $projects = $query->execute()->fetchAll();
      
    $rows = array();
    
  
  
    foreach($projects as $project) {
      $project->info = unserialize($project->info);
      if(isset($project->info['hidden']) && $project->info['hidden']){
        continue;
      }
  
      $status = array(
        'data' => '  [x]  ',
        '#color' => 'red',
      );
      if($project->status){
        $status = array(
          'data' => '  [v]  ',
          '#color' => 'green',
        );
      }
      
      $name = array(
          'data' => $project->name,
          '#color' => 'bold_blue',
        );
      $rows[] = array(
        $name,
        $project->info['name'],
        $project->type,
        $status,
        $project->info['version'],
      );
      
    }
  }
  
  if(!isset($options['type']) or $options['type'] == 'layout') {
    $used_layouts = layout_load_all();
  
    $layouts = layout_get_layout_template_info();
    foreach($layouts as $layout) {
      $name = array(
        'data' => $layout['name'],
        '#color' => 'bold_blue',
      );
      
      $status = FALSE;
      foreach($used_layouts as $used_layout){
        if($used_layout->layout_template == $layout['name']) {
          $status = array(
            'data' => '  [v]  ',
            '#color' => 'green',
          );
        }
      }
      if(!$status) {
        $status = array(
          'data' => '  [-]  ',
          '#color' => 'red',
        );
      }
      
      $rows[] = array(
        $name,
        $layout['title'],
        'layout',
        $status,
        $layout['version'],
      );
    }
  }
  $header = array(
    'Project',
    'Name',
    'Type',
    'Status',
    'Version',
  );

  return array(
    '#type' => 'table',
    'rows' => $rows,
    'header' => $header,
    'delimiter' => '  ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );
}

function modules_b_enable_callback($arguments, $options) {
  print_r($arguments);
  print_r($options);
}
