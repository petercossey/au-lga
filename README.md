Australian LGAs
======

Drupal 7 module exposing Australian LGA for given geopoint.

You give it coordinates, it gives you infor about the LGA.

Requirements
------

You need up-to-date version of geoPHP library: one that has Polygon:pointInPolygon method.

You need dBase PHP extension enabled for importing LGAs data. See http://php.net/manual/en/book.dbase.php for details.

Installation
------

After installing the module as usual, run drush ali to import data.

Usage
-----

Make a request to /js/au_lga/geocode/[lat]/[lon], info about the LGA will be returned as JSON.

Optionally you can use https://www.drupal.org/project/js to speed up the response.
