<?php
/**
 * @file
 * Command(s) for working with node.
 */

/**
 * Implements hook_bee_command().
 */
function node_bee_command() {
  return array(
    'nodes' => array(
      'description' => bt('List all nodes.'),
      'callback' => 'node_bee_callback',
      'group' => 'node',
      'aliases' => array('nls', 'node-list'),
      'options' => array(
        'nid' => array(
          'description' => bt("Get content of a  node."),
          'value' => bt('nid'),
        ),
      ),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee node' => bt('Display a list of all nodese.'),
      ),
    ),
    'node-update' => array(
      'description' => bt('Add or update a node.'),
      'callback' => 'node_update_bee_callback',
      'group' => 'node',
      'options' => array(
        'nid' => array(
          'description' => bt("Give nid of the node to change. (int)"),
          'value' => bt('nid'),
        ),
        'uid' => array(
          'description' => bt("Give uid for the node, add or change. (int)"),
          'value' => bt('uid'),
        ),
        'type' => array(
          'description' => bt("Give node type of the node, add or change. (string)"),
          'value' => bt('type'),
        ),
        'title' => array(
          'description' => bt("Give the title  of the node, add or change. (string)"),
          'value' => bt('title'),
        ),
        'body' => array(
          'description' => bt("Give the text body  of the node, add or change. (string)"),
          'value' => bt('body'),
        ),
        'file' => array(
          'description' => bt("Give the filename to upload text for the body of the node, add or change. (string)"),
          'value' => bt('file'),
        ),
        'status' => array(
          'description' => bt("Publish the node. (0/1)"),
          'value' => bt('status'),
        ),
        'promote' => array(
          'description' => bt("Promote or place to homepage. (0/1)"),
          'value' => bt('promote'),
        ),
        'sticky' => array(
          'description' => bt("Set sticky of the node, add or change. (0/1)"),
          'value' => bt('sticky'),
        ),
      ),
      'aliases' => array('nu'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee node-update --title="This is a new page"' => bt("Add node with title 'This is a new page'."),
        'bee node-update --title="This is a new page" --body="This a test"' => bt("Add node with title 'This is a new page' with the body content 'This a test'."),
        'bee node-update --title="This is a new page" --file="/path/file.html"' => bt("Add node with title 'This is a new page' with the body content from file /path/file.html."),
        'bee node-update --nid=1 --title="This is a updated page"' => bt("Change  node 2 to title 'This is a updated page'."),
      ),
    ),
    'node-delete' => array(
      'description' => bt('Delete a node.'),
      'callback' => 'node_delete_bee_callback',
      'group' => 'node',
      'aliases' => array('ndel'),
      'options' => array(
        'nid' => array(
          'description' => bt("Give nid of the node to delete. (int)"),
          'value' => bt('NID'),
        ),
      ),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee node-delete --nid=3' => bt("Delete node 3."),
      ),
    ),
    'menus' => array(
      'description' => bt('List all menus.'),
      'callback' => 'menu_bee_callback',
      'group' => 'node',
      'aliases' => array('menu-list'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee menus' => bt('Display a list of all links.'),
      ),
    ),
    'menu-update' => array(
      'description' => bt('Update or create a menu link.'),
      'callback' => 'menu_update_bee_callback',
      'group' => 'node',
      'aliases' => array('mu'),
      'options' => array(
        'nid' => array(
          'description' => bt("(required for new link) Node id (int)"),
          'value' => bt('nid'),
        ),
        'menu' => array(
          'description' => bt("(optional) Menu name, otherwise default to main-menu.(string)"),
          'value' => bt('menu'),
        ),
        'title' => array(
          'description' => bt("(optional) Menu title, otherwise use the title of the node. (string) "),
          'value' => bt('title'),
        ),
        'mlid' => array(
          'description' => bt("(optional) Menu link identifier, the primary integer key for each menu link. Can be set to an existing value, or to 0 or empty to insert a new link. (int)"),
          'value' => bt('mlid'),
        ),
        'plid' => array(
          'description' => bt("(optional) The mlid for the parent. (int)"),
          'value' => bt('plid'),
        ),
        'hide' => array(
          'description' => bt("(optional) Hide menu link (0/1)"),
          'value' => bt('hide'),
        ),
        'weight' => array(
          'description' => bt("(optional) Integer to determine position in menu. Default is 0. (int)"),
          'value' => bt('weight'),
        ),
      ),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee menu-update --menu=main-menu --nid=10' => bt('Add node 10 to menu main-menu'),
        'bee mu  --mlid=363 --plid=355 --weight=10 --title=\'Yet anthor test page\'' => bt('Move link 363 to the link 355 as parent with title "Yet anthor test page" and move to position 10'),
      ),
    ),
    'menu-delete' => array(
      'description' => bt('Delete a menu link.'),
      'callback' => 'menu_delete_bee_callback',
      'group' => 'node',
      'aliases' => array('md'),
      'options' => array(
        'mlid' => array(
          'description' => bt("(one of required) Give the mlid of the menu to delete. (int)"),
          'value' => bt('mlid'),
        ),
        'path' => array(
          'description' => bt("(one of required) Give the path  of the menu to delete. (string)"),
          'value' => bt('path'),
        ),
      ),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee menu-delete --mlid=300' => bt("Delete menu link 300"),
      ),
    ),
  );
}

/**
 * Command callback: List all nodes or one.
 */
function node_bee_callback($arguments, $options) {
  if (!empty($options['nid'])) {
    $node = node_load($options['nid']);
    bee_message(bt("The !node node: !title !text", array(
      '!title' => $node->title,
      '!text' => "\n".preg_replace('/\<br(\s*)?\/?\>/i', "\n", $node->body['und'][0]['value']),
      '!node' => $options['nid'],
    )));

  }
  else {
    $rows = array();
    $type = "";
    $nodes = node_load_multiple(FALSE);
    foreach ($nodes as $node) {
      $rows[] = array(

      array('value' => $node->nid),
      array('value' => $node->title),
      array('value' => $node->type),
      array('value' => (user_load($node->uid)->name)),
      array('value' => ($node->status > 0) ? bt('Published') : bt('Not published')),
      array('value' => format_date($node->changed)),
      );
    }

    $header = array(
      array('value' => bt('Nid')),
      array('value' => bt('Title')),
      array('value' => bt('Type')),
      array('value' => bt('Name')),
      array('value' => bt('Status')),
      array('value' => bt('Changed')),
    );
    return array(
      array(
        'type' => 'table',
        'variables' => array(
          'rows' => $rows,
          'header' => $header,
        ),
      ),
    );
  }
}

/**
 * Command callback: Add or update a node
 */
function node_update_bee_callback($arguments, $options) {
  if (!empty($options['type'])) {
    $node_types = array_keys(node_type_get_names());
    if (!in_array($options['type'], $node_types)) {
      $node_types = implode(', ', $node_types);
      bee_message(bt("!type node type name doesn't exists!, types available: !types.", array(
        '!type' => $options['type'],
        '!types' => $node_types,
      )), 'error');
      return;
    }
  }
  if (!empty($options['file'])) {
    if (file_exists($options['file'])) {
      $body = file_get_contents($options['file']);
    }
    else {
      bee_message(bt("!file file doesn't exists!.", array(
        '!file' => $options['file'],
      )), 'error');
      return;
    }
  }
  elseif (!empty($options['body'])) {
    $body = $options['body'];
  }
  else {
    $body = '';
  }

  if (!empty($options['nid'])) {
    $node = node_load($options['nid']);
    if (!empty($node)) {
      $node->uid = !empty($options['uid']) ? $options['uid'] : $node->uid;
      $node->type = !empty($options['type']) ? $options['type'] : $node->type;
      $node->title = !empty($options['title']) ? $options['title'] : $node->title;
      $node->promote = !empty($options['promote']) ? $options['promote'] : $node->promote;
      $node->sticky = !empty($options['sticky']) ? $options['sticky'] : $node->sticky;
      $node->status = !empty($options['status']) ? $options['status'] : $node->status;
      $node->changed = REQUEST_TIME;
      $node->body['und'][0]['value'] = !empty($body) ? $body : $node->body['und'][0]['value'];
      $node->save();
    }
    else {
        bee_message(bt("!nid node doesn't exists!.", array(
          '!nid' => $options['nid'],
        )), 'error');
      return;
    }
  }
  else {
    if (!isset($options['title'])) {
      bee_message(bt("Title option missing."), 'error');
      return;
    }
    $node = entity_create('node', array(
      'uid' => !empty($options['uid']) ? $options['uid'] : 1,
      'type' => !empty($options['type']) ? $options['type'] : 'page',
      'title' => $options['title'],
      'promote' => !empty($options['promote']) ? $options['promote'] : 0,
      'sticky' => !empty($options['sticky']) ? $options['sticky'] : 0,
      'status' => !empty($options['status']) ? $options['status'] : 1,
      'langcode' => LANGUAGE_NONE,
      'created' => REQUEST_TIME,
      'changed' => REQUEST_TIME,
      'body' => array (
        'und' => array (
          0 => array (
            'value' => $body,
            'summary' => !empty($options['summary']) ? $options['summary'] : '',
            'format' => !empty($options['format']) ? $options['format'] : 'filtered_html',
          ),
        ),
      ),
    ));

    $node->save();
    if ( !empty($node->nid)) {
      bee_message(bt("!node node has been created.", array(
        '!node' => $options['title'],
      )), 'success');
    }
    else {
      bee_message(bt("!node node creation failed!", array(
        '!title' => $options['title'],
      )), 'error');
    }
  }
}

/**
 * Command callback: Delete a node
 */
function node_delete_bee_callback($arguments, $options) {
  if (!empty($options['nid'])) {
    $node = node_load($options['nid']);
    if (!$node) {
      bee_message(bt("!nid node doesn't exists!.", array(
        '!nid' => $options['nid'],
      )), 'error');
      return;
    }
      $answer = bee_confirm(bt("Are you sure to delete the node !nid, !title?", array(
        '!nid' => $options['nid'],
        '!title' => $node->title,
      )), FALSE);
    if (!$answer) {
      return;
    }
      node_delete($options['nid']);
      $node = node_load($options['nid']);
    if (empty($node)) {
      bee_message(bt("!nid node is deleted.", array(
        '!nid' => $options['nid'],
      )), 'success');
    }
    else {
      bee_message(bt("!nid node unable to delete!.", array(
        '!nid' => $options['nid'],
      )), 'error');
    }
  }
  else {
      bee_message(bt("!nid node doesn't exists!.", array(
        '!nid' => $options['nid'],
      )), 'error');
    return;
  }
}

/**
 * Command callback:  list all menu links ordered by weight
 */
function menu_bee_callback($arguments, $options) {
  $rows = array();
  foreach (menu_get_menus() as $menu => $name) {
    $tree = menu_tree_all_data($menu);
    $rowsmenu = _output_menu_nested_tree($tree);
    if (!empty($rowsmenu) && is_array($rowsmenu)) {
      $rows = array_merge($rows, $rowsmenu);
    }
    $rows[] = array(
       array('value' => ''),
       array('value' => ''),
       array('value' => ''),
       array('value' => ''),
       array('value' => ''),
    );
  }
  $header = array(
    array('value' => bt('Menu')),
    array('value' => bt('Weight')),
    array('value' => bt('Link')),
    array('value' => bt('Title')),
    array('value' => bt('Hidden')),
  );
  return array(
    array(
      'type' => 'table',
      'variables' => array(
        'rows' => $rows,
        'header' => $header,
      ),
    ),
  );
}

/**
 * Command callback: add/update menu links.
 */
function menu_update_bee_callback($arguments, $options) {
  if (!empty($options['mlid'])) {
    $item = menu_link_load($options['mlid']);
    $item['link_path'] = !empty($options['link_path']) ? $options['link_path'] : $item['link_path'];
    $item['link_title'] = !empty($options['title']) ? $options['title'] : $item['link_title'];
    $item['menu_name'] = !empty($options['menu']) ? $options['menu'] : $item['menu_name'];
    $item['weight'] = !empty($options['weight']) ? $options['weight'] : $item['weight'];
    $item['plid'] = !empty($options['plid']) ? $options['plid'] : $item['plid'];
    if ( menu_link_save($item)) {
      bee_message(bt("!menu menu has been updated", array(
        '!menu' => $options['mlid'],
      )), 'success');
      menu_rebuild();
    }
    else {
      bee_message(bt("!menu updated failed!.", array(
        '!menu' => $options['mlid'],
      )), 'error');
    }
  }
  else {
    if (!empty($options['nid'])) {
      $node = node_load($options['nid']);
      if (!$node) {
        bee_message(bt("!nid node doesn't exists!.", array(
          '!nid' => $options['nid'],
        )), 'error');
        return;
      }
      $node = node_load($options['nid']);
      $item = array( 'link_path' => 'node/' . $node->nid,
        'link_title' => !empty($options['title']) ? $options['title'] : $node->title,
        'menu_name' => !empty($options['menu']) ? $options['menu'] : '',
        'weight' => !empty($options['weight']) ? $options['weight'] : 0,
        'plid' => !empty($options['mlid']) ? $options['mlid'] : '',
        'mlid' => !empty($options['mlid']) ? $options['mlid'] : '',
      );
      if ( $mlid = menu_link_save($item)) {
        menu_rebuild();
        bee_message(bt("Menu has been created"), 'success');
      }
      else {
        bee_message(bt("!menu failed!.", array(
          '!menu' => $item['link_title'],
        )), 'error');
      }
    }
    else {
        bee_message(bt("!nid node doesn't exists!.", array(
          '!nid' => $options['nid'],
        )), 'error');
      return;
    }
  }
}

/**
 * Command callback: Delete a menu link or links.
 */
function menu_delete_bee_callback($arguments, $options) {
  if (!empty($options['mlid']) && empty($options['path'])) {
    if (menu_link_delete($options['mlid'], $PATH = NULL)) {
      bee_message(bt("Menu has been deleted"), 'success');
    }
  }
  elseif (!empty($options['path']) && empty($options['mlid'])) {
    if (menu_link_delete($mlid = NULL, $options['path'])) {
      bee_message(bt("Menu has been deleted"), 'success');
    }
  }
  else {
    bee_message(bt("Either option mlid or path", array(
    )), 'error');
  }
}

/**
 * Sub function for listing menu links.
 */
function _output_menu_nested_tree($tree,$parent=FALSE) {
  if (count($tree)) {
    foreach ($tree as $item) {

      $menu_link = $item['link'];
      if (empty($parent)) {
        $link_path = $menu_link['link_path'].'('.$menu_link['mlid'].')';
      }
      else {
        $link_path = $parent.$menu_link['link_path'].'('.$menu_link['mlid'].')';
      }
      $rows[] = array(
        array('value' => $menu_link['menu_name']),
        array('value' => $menu_link['weight']),
        array('value' => $link_path),
        array('value' => $menu_link['title']),
        array('value' => $menu_link['hidden']),
      );
      if (!empty($item['below'])) {
        if (empty($parent)) {
          $parent = $menu_link['link_path'].'('.$menu_link['mlid'].')->/';
        }
        else {
          $parent .= $menu_link['link_path'].'('.$menu_link['mlid'].')->/';
        }
        $rowschild = _output_menu_nested_tree($item['below'], $parent);
        if (!empty($rowschild) && is_array($rowschild)) {
          $rows = array_merge($rows, $rowschild);
        }
      }
      if (empty($item['below'][0])) {
          unset($parent);
      }
    }
  }
  if (!empty($rows) && is_array($rows)) {
    asort($rows);
    return($rows);
  }
  else {
    return;
  }
}

