import RaylibCanvas from "./components/RaylibCanvas";
import Hero from "./components/Hero.jsx"
import "./style.css";

export default function App() {
    return (
        <>
            <RaylibCanvas />

            <main className="site-content">
                <Hero />
            </main>
        </>
    );
}

