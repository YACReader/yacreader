(function () {
  "use strict";

  var root = document.documentElement;

  function preferredTheme() {
    try {
      var savedTheme = localStorage.getItem("yacreader-server-theme");
      if (savedTheme === "light" || savedTheme === "dark") {
        return savedTheme;
      }
    } catch (error) {
    }

    return window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
  }

  function applyTheme(theme) {
    root.dataset.theme = theme;

    var toggle = document.querySelector("[data-theme-toggle]");
    if (toggle) {
      toggle.setAttribute("aria-label", theme === "dark" ? "Use light theme" : "Use dark theme");
    }
  }

  applyTheme(preferredTheme());

  document.addEventListener("DOMContentLoaded", function () {
    var toggle = document.querySelector("[data-theme-toggle]");
    if (toggle) {
      toggle.addEventListener("click", function () {
        var theme = root.dataset.theme === "dark" ? "light" : "dark";
        applyTheme(theme);

        try {
          localStorage.setItem("yacreader-server-theme", theme);
        } catch (error) {
        }
      });
    }

    document.querySelectorAll("[data-controls]").forEach(function (checkbox) {
      var control = document.getElementById(checkbox.dataset.controls);
      if (!control) {
        return;
      }

      function updateControl() {
        control.disabled = !checkbox.checked;
      }

      checkbox.addEventListener("change", updateControl);
      updateControl();
    });
  });
}());
