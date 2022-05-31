// Function to handle the menu when on small devices
function add_responsive_to_nav() {
    const nav = document.getElementById("top-nav")
    if (nav.className == "top-navigation") {
        nav.className += " responsive"
    } else {
        nav.className = "top-navigation"
    }
}