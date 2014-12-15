<?php
/**
 * @file
 * Importing LGAs info.
 */

define("AU_LGA_EOF", 0);

/**
 * States.
 */
function au_lga_states() {
  return array(
    1 => array(
      'short' => 'NSW',
      'long' => 'New South Wales',
      'type' => 'state',
    ),
    2 => array(
      'short' => 'VIC',
      'long' => 'Victoria',
      'type' => 'state',
    ),
    3 => array(
      'short' => 'QLD',
      'long' => 'Queensland',
      'type' => 'state',
    ),
    4 => array(
      'short' => 'SA',
      'long' => 'South Australia',
      'type' => 'state',
    ),
    5 => array(
      'short' => 'WA',
      'long' => 'Western Australia',
      'type' => 'state',
    ),
    6 => array(
      'short' => 'TAS',
      'long' => 'Tasmania',
      'type' => 'state',
    ),
    7 => array(
      'short' => 'NT',
      'long' => 'Northern Territory',
      'type' => 'territory',
    ),
    8 => array(
      'short' => 'ACT',
      'long' => 'Australian Capital Territory',
      'type' => 'territory',
    ),
    9 => array(
      'short' => 'OT',
      'long' => 'Other Territories',
      'type' => 'territory',
    ),
  );
}

/**
 * Import states.
 */
function au_lga_import_states() {
  $map = array();
  $states_vocab = taxonomy_vocabulary_machine_name_load('au_lga_state');


  if ($states_vocab) {
    // Delete old states.
    $query = new EntityFieldQuery();
    $result = $query
      ->entityCondition('entity_type', 'taxonomy_term')
      ->propertyCondition('vid', $states_vocab->vid)
      ->execute();
    if (!empty($result['taxonomy_term'])) {
      $tids = array_keys($result['taxonomy_term']);
      entity_delete_multiple('taxonomy_term', $tids);
    }

    // Create new states.
    $states = au_lga_states();
    foreach ($states as $id => $state) {
      $term = new stdClass();
      $term->name = $state['short'];
      $term->description = $state['long'];
      $term->vid = $states_vocab->vid;
      $term->field_au_lga_state_type[LANGUAGE_NONE][0]['value'] = $state['type'];
      taxonomy_term_save($term);
      $map[$id] = $term->tid;
    }
  }
  return $map;
}

/**
 * Import LGAs.
 */
function au_lga_import_lga() {
  // Delete old LGAs.
  au_lga_delete_lga();

  // Create new LGAs.
  $states = au_lga_import_states();

  // Open data file.
  $shp = au_lga_open_data();

  // Load geoPHP classes.
  geophp_load();

  // Process nodes.
  $i = 0;
  while (au_lga_create_next_lga($shp, $states) !== AU_LGA_EOF) {
    $i++;
  }
}

/**
 * Deletes all LGAs.
 */
function au_lga_delete_lga() {
  $query = new EntityFieldQuery();
  $result = $query
    ->entityCondition('entity_type', 'node')
    ->propertyCondition('type', 'au_lga')
    ->execute();
  if (!empty($result['node'])) {
    $nids = array_keys($result['node']);
    entity_delete_multiple('node', $nids);
  }
}

/**
 * Open LGAs data file.
 */
function au_lga_open_data($fpos = NULL) {
  module_load_include('inc', 'au_lga', 'lib/ShapeFile');
  $path = drupal_get_path('module', 'au_lga') . '/data/SLA11aAust.shp';
  $options = array('noparts' => false);
  if ($fpos) {
    $options['fpos'] = $fpos;
  }
  return new ShapeFile($path, $options); // along this file the class will use file.shx and file.dbf
}

/**
 * Create LGA node.
 */
function au_lga_create_next_lga($shp, $states) {
  global $user;
  if ($record = $shp->getNext()) {

    $shp_info = $record->getShpData(); // All the data related to the poligon
    $dbf_info = $record->getDbfData(); // The alphanumeric information related to the figure

    // Skip deleted.
    if (!empty($dbf_info['deleted'])) {
      $node = NULL;
    }
    else {
      // Node frame.
      $node = new stdClass();
      $node->type = "au_lga";
      $node->language = LANGUAGE_NONE; // Or e.g. 'en' if locale is enabled

      $node->uid = $user->uid;
      $node->status = 1; //(1 or 0): published or not
      $node->promote = 0; //(1 or 0): promoted to front page
      $node->comment = 0; // 0 = comments disabled, 1 = read only, 2 = read/write

      // Title and type.
      $title_info = trim($dbf_info['LGA_NAME11']);
      if (preg_match('/(.+?)\s*+\((.+?)\)\s*+(.*)?$/', $title_info, $matches)) {
        $node->title = trim($matches[1]);
        $node->field_au_lga_type[LANGUAGE_NONE][0]['value'] = trim($matches[2]);
        if (isset($matches[3]) && $matches[3]) {
          $node->title .= ' ' . trim($matches[3]);
        }
      }
      else {
        $node->title = $title_info;
      }

      // Codes.
      $node->field_au_lga_code[LANGUAGE_NONE][0]['value'] = $dbf_info['LGA_CODE11'];

      // State.
      if (isset($states[$dbf_info['STATE_CODE']])) {
        $node->field_au_lga_state[LANGUAGE_NONE][0]['tid'] = $states[$dbf_info['STATE_CODE']];
      }

      // Regions.
      foreach ($shp_info['parts'] as $part) {
        $points = array();
        foreach ($part['points'] as $point_info) {
          $points[] = $point_info['x'] . ' ' . $point_info['y'];
        }
        $region = geoPHP::load('POLYGON((' . implode(', ', $points) . '))', 'wkt');
        $node->field_au_lga_region[LANGUAGE_NONE][] = geofield_get_values_from_geometry($region);
      }

      // Save node.
      node_object_prepare($node); // Sets some defaults. Invokes hook_prepare() and hook_node_prepare().
      $node = node_submit($node); // Prepare node for saving
      node_save($node);
    }
    return array(
      'node' => $node,
      'record' => $record
    );
  }
  return AU_LGA_EOF;
}