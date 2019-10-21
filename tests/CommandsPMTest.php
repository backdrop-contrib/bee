<?php
/**
 * @file
 * PHPUnit tests for Backdrop Console PM commands.
 */

use PHPUnit\Framework\TestCase;

class CommandsPMTest extends TestCase {

  /**
   * Check that the PM Download command works.
   */
  public function testPmDownload() {
    // Single module.
    $output_single = shell_exec('b dl simplify');
    $this->assertStringContainsString('simplify downloaded to', $output_single);
    $this->assertTrue(file_exists('modules/simplify/simplify.info'));

    // Multiple projects: theme and layout.
    $output_multiple = shell_exec('b dl borg hero');
    $this->assertStringContainsString('borg downloaded to', $output_multiple);
    $this->assertTrue(file_exists('themes/borg/borg.info'));
    $this->assertStringContainsString('hero downloaded to', $output_multiple);
    $this->assertTrue(file_exists('layouts/hero/hero.info'));

    // Backdrop core.
    $output_backdrop = shell_exec('cd ../ && b dl backdrop');
    $this->assertStringContainsString('backdrop downloaded to', $output_backdrop);
    $this->assertTrue(file_exists('../backdrop/index.php'));
    $output_backdrop_root = shell_exec('b dl backdrop --root=../bd_root');
    $this->assertStringContainsString('backdrop downloaded to', $output_backdrop_root);
    $this->assertTrue(file_exists('../bd_root/index.php'));

    // Remove downloads for future tests.
    exec('rm -r modules/simplify themes/borg layouts/hero ../backdrop ../bd_root');
  }

  /**
   * Check that the PM List command works.
   */
  public function testPmList() {
    $output_all = shell_exec('b pml');
    $this->assertStringContainsString('Administration Bar', $output_all);
    $this->assertStringContainsString('Bartik', $output_all);
    $this->assertStringContainsString('Sutro', $output_all);

    $output_module = shell_exec('b pml --type=module');
    $this->assertStringContainsString('Update Manager', $output_module);
    $this->assertStringNotContainsString('Bartik', $output_module);
    $this->assertStringNotContainsString('Sutro', $output_module);

    $output_theme = shell_exec('b pml --type=theme');
    $this->assertStringContainsString('Basis', $output_theme);
    $this->assertStringNotContainsString('Administration Bar', $output_theme);
    $this->assertStringNotContainsString('Sutro', $output_theme);

    $output_layout = shell_exec('b pml --type=layout');
    $this->assertStringContainsString('Moscone Flipped', $output_layout);
    $this->assertStringNotContainsString('Administration Bar', $output_layout);
    $this->assertStringNotContainsString('Bartik', $output_layout);
  }

  /**
   * Check that the PM Enable module command works.
   */
  public function testPmEnableModule() {
    $output_single = shell_exec('b en --y contact');
    $this->assertStringContainsString('Module contact enabled', $output_single);

    $output_multiple = shell_exec('b en --y language locale');
    $this->assertStringContainsString('Module language enabled', $output_multiple);
    $this->assertStringContainsString('Module locale enabled', $output_multiple);
  }

  /**
   * Check that the PM Disable module command works.
   */
  public function testPmDisableModule() {
    $output_single = shell_exec('b dis --y contact');
    $this->assertStringContainsString('Module contact disabled', $output_single);

    $output_multiple = shell_exec('b dis --y language locale');
    $this->assertStringContainsString('Module language disabled', $output_multiple);
    $this->assertStringContainsString('Module locale disabled', $output_multiple);
  }

  /**
   * Check that the PM Uninstall module command works.
   */
  public function testPmUninstallModule() {
    $output_single = shell_exec('b pmu --y contact');
    $this->assertStringContainsString('Module contact uninstalled', $output_single);

    $output_multiple = shell_exec('b pmu --y language locale');
    $this->assertStringContainsString('Module language uninstalled', $output_multiple);
    $this->assertStringContainsString('Module locale uninstalled', $output_multiple);
  }

  /**
   * Check that the PM Enable theme command works.
   */
  public function testPmEnableTheme() {
    exec('b dl atomium');
    exec('b dl tatsu');
    exec('b dl purple');

    $output_normal = shell_exec('b en --y atomium');
    $this->assertStringContainsString('Theme atomium enabled', $output_normal);

    $output_default = shell_exec('b en --y tatsu --default');
    $this->assertStringContainsString('Theme tatsu enabled', $output_default);

    $output_admin = shell_exec('b en --y purple --admin');
    $this->assertStringContainsString('Theme purple enabled', $output_admin);
  }

  /**
   * Check that the PM Disable theme command works.
   */
  public function testPmDisableTheme() {
    $output = shell_exec('b dis --y atomium tatsu purple');
    $this->assertStringContainsString('Theme atomium disabled', $output);
    $this->assertStringContainsString('Theme tatsu disabled', $output);
    $this->assertStringContainsString('Theme purple disabled', $output);

    // Remove themes for future tests.
    exec('rm -r themes/atomium themes/tatsu themes/purple');
  }

}
