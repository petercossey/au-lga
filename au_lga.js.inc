<?php
/**
 * @file
 * JS callbacks of au_lga module.
 */

/**
 * Returns LGA data for given coordinates.
 */
function au_lga_geocode_callback($y, $x) {
  $info = array();
  if (is_numeric($x) && is_numeric($y)) {
    geophp_load();

    $result = db_query('SELECT f.entity_id AS entity_id, f.field_au_lga_region_geom AS field_au_lga_region_geom FROM {field_data_field_au_lga_region} f WHERE (:y BETWEEN field_au_lga_region_bottom AND field_au_lga_region_top) AND (:x BETWEEN field_au_lga_region_left AND field_au_lga_region_right )', array(':x' => $x, ':y' => $y));

    $nids = array();
    if ($result) {
      $point = new Point($x, $y);
      $types = list_allowed_values(field_info_field('field_au_lga_type'));
      foreach ($result as $record) {
        if (in_array($record->entity_id, $nids)) {
          continue;
        }
        $polygon = geoPHP::load($record->field_au_lga_region_geom, 'wkb');
        if ($polygon->pointInPolygon($point)) {
          $nids[] = $record->entity_id;
          $node = node_load($record->entity_id);
          $type = check_plain($types[$node->field_au_lga_type[LANGUAGE_NONE][0]['value']]);
          $item = array(
            'name' => check_plain($node->title),
            'full_name' => $type . ' of ' . check_plain($node->title),
            'type' => $types[$node->field_au_lga_type[LANGUAGE_NONE][0]['value']],
            'code' => check_plain($node->field_au_lga_code[LANGUAGE_NONE][0]['value']),
            '5digit' => check_plain($node->field_au_lga_5digit[LANGUAGE_NONE][0]['value']),
            'reg' => check_plain($node->field_au_lga_reg[LANGUAGE_NONE][0]['value']),
            'state' => array(),
          );

          $state = taxonomy_term_load($node->field_au_lga_state[LANGUAGE_NONE][0]['tid']);
          if ($state) {
            $item['state'] = array(
              'short' => check_plain($state->name),
              'long' => check_plain($state->description),
              'type' => check_plain($state->field_au_lga_state_type[LANGUAGE_NONE][0]['value']),
            );
          }
          $info[] = $item;
        }
      }
    }
  }
  drupal_json_output($info);
}
